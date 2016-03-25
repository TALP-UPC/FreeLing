
#include <fstream>  
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/tagset.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

#define MOD_TRACENAME L"TAGSET"
#define MOD_TRACECODE TAGGER_TRACE

using namespace std;

namespace freeling {

  //////////////////////////////////////////////////////////////////////
  /// --- constructor: load given file
  //////////////////////////////////////////////////////////////////////

  tagset::tagset(const wstring &ftagset) : PAIR_SEP(L"="), MSD_SEP(L"|") {

    // configuration file
    enum sections {DIRECT_TRANSLATIONS, DECOMPOSITION_RULES};
    config_file cfg;  
    cfg.add_section(L"DirectTranslations",DIRECT_TRANSLATIONS);
    cfg.add_section(L"DecompositionRules",DECOMPOSITION_RULES);
  
    if (not cfg.open(ftagset))
      ERROR_CRASH(L"Error opening file "+ftagset);

    wstring line;
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {
      case DIRECT_TRANSLATIONS: {
        // reading direct tag translation
        wistringstream sin;
        sin.str(line);
        wstring tag, shtag, msd;
        sin>>tag>>shtag>>msd;
        direct[tag] = make_pair(shtag,msd); 
        direct_inv[util::wstring2set(msd,MSD_SEP)] = tag;
        TRACE(3,L"Read direct translation for "+tag+L" = ("+shtag+L","+msd+L")");
        break;
      }

      case DECOMPOSITION_RULES: { 
        // reading a decomposition rule
        wistringstream sin;
        sin.str(line);
        wstring cat, shsz, pos; 
        
        // read category code, short tag size, and category name
        sin>>cat>>shsz>>pos;
        name[cat] = pos;
        name_inv[pos] = cat;
        
        shtag_size[cat] = list<int>();
        list<wstring> sz = util::wstring2list(shsz,L",");
        for (list<wstring>::iterator k=sz.begin(); k!=sz.end(); k++) 
          shtag_size[cat].push_back(util::wstring2int(*k));

        TRACE(3,L"Read short tag size for "+cat+L" ("+pos+L") = "+util::int2wstring(shsz));
        
        // other fields are features
        int i=1;
        wstring msd;
        while (sin>>msd) {
          wstring key = cat+L"#"+util::int2wstring(i);
          
          vector<wstring> k=util::wstring2vector(msd,L"/");  // separate feature name/values "postype/C:common;P:proper"
          feat[key] = k[0];    // store name (e.g. "postype") for this position
          feat[cat+L"#"+k[0]]=util::int2wstring(i); // store postition for this name (used for cross check only)
          vector<wstring> v=util::wstring2vector(k[1],L";");  // separate values "C:common;P:proper"
          for (size_t j=0; j<v.size(); j++) {
            vector<wstring> t=util::wstring2vector(v[j],L":"); // split value code:name "C:common"
            val[key+L"#"+freeling::util::uppercase(t[0])] = t[1];
            val_inv[key+L"#"+t[1]] = freeling::util::uppercase(t[0]);
          }
          
          i++;
        }
        break;
      }

      default: break;
      }
    }

    cfg.close();

    TRACE(1,L"Module created successfully.");
  }


  //////////////////////////////////////////////////////////////////////
  /// --- destructor
  //////////////////////////////////////////////////////////////////////

  tagset::~tagset() {}



  //////////////////////////////////////////////////////////////////////
  /// get short version of given tag
  //////////////////////////////////////////////////////////////////////

  wstring tagset::get_short_tag(const wstring &tag) const {

    TRACE(6,L"get short tag for "+tag);
    // if direct translation exists, get it 
    map<wstring,pair<wstring,wstring> >::const_iterator p=direct.find(tag);
    if (p!=direct.end()) {
      TRACE(6,L"  Found direct entry "+p->second.first);
      return p->second.first;
    }
    else {
      // no direct value, compute short tag cutting n first positions (n==0 -> all)
      map<wstring,list<int> >::const_iterator s=shtag_size.find(tag.substr(0,1));
      if (s!=shtag_size.end()) {

        if (s->second.size()==1) {
          int ln = (*s->second.begin());
          TRACE(6,L"   cuting first positions sz="+util::int2wstring(ln)+L" of "+tag);
          return (ln==0 ? tag : tag.substr(0,ln));
        }
        else {
          wstring shtg=L"";
          for (list<int>::const_iterator k=s->second.begin(); k!=s->second.end(); k++) {
            if (*k < int(tag.length()))
              shtg.append(1,tag[*k]);
            else {
              WARNING(L"Tag '"+tag+L"' too short for requested digits. Unchanged.");              
              return tag;
            }
          }
          TRACE(6,L"   Extracting digits "+shtg+L" from "+tag);
          return shtg;
        }
      }
    }

    // we don't know anything about this tag
    WARNING(L"No rule to get short version of tag '"+tag+L"'.");
    return tag;
  }


  //////////////////////////////////////////////////////////////////////
  /// get map of <feature,value> pairs with morphological information
  //////////////////////////////////////////////////////////////////////

  map<wstring,wstring> tagset::get_msd_features_map(const wstring &tag) const {

    map<wstring,wstring> msd;
    list<pair<wstring,wstring> > lm = get_msd_features(tag);
    for (list<pair<wstring,wstring> >::const_iterator m=lm.begin(); m!=lm.end(); m++) 
      msd.insert(*m);

    return msd;
  }


  //////////////////////////////////////////////////////////////////////
  /// get list of <feature,value> pairs with morphological information
  //////////////////////////////////////////////////////////////////////

  list<pair<wstring,wstring> > tagset::get_msd_features(const wstring &tag) const {

    TRACE(6,L"get msd for "+tag);
    map<wstring,pair<wstring,wstring> >::const_iterator p=direct.find(tag);
    if (p!=direct.end())
      return util::wstring2pairlist(p->second.second,PAIR_SEP,MSD_SEP);
    else 
      return compute_msd_features(tag);
  }


  //////////////////////////////////////////////////////////////////////
  /// get list <feature,value> pairs with morphological information, in a string format
  //////////////////////////////////////////////////////////////////////

  wstring tagset::get_msd_string(const wstring &tag) const {

    TRACE(6,L"get msd string for "+tag);
    map<wstring,pair<wstring,wstring> >::const_iterator p=direct.find(tag);
    if (p!=direct.end()) 
      return p->second.second;
    else
      return util::pairlist2wstring(compute_msd_features(tag),PAIR_SEP,MSD_SEP);
  }


  //////////////////////////////////////////////////////////////////////
  /// private method to decompose the tag into morphological features
  /// interpreting each digit in the tag according to field definition.
  /// feat[<cat,i>] is the feature name (e.g. "postype")
  /// val[<cat,i>] is a map<code,name> with the feature values
  //////////////////////////////////////////////////////////////////////

  list<pair<wstring,wstring> > tagset::compute_msd_features(const wstring &tag) const {

    TRACE(6,L"computing msd for "+tag);

    wstring cat = tag.substr(0,1);  // get category code (N, A, V...)
    list<pair<wstring,wstring> > res;

    map<wstring,wstring>::const_iterator nm=name.find(cat);
    if (nm==name.end()) {
      WARNING(L"Unknown category for tag "+cat);

      return res;
    }
    res.push_back(make_pair(L"pos",nm->second)); // category name (noun, adjective, verb, etc)

    for (size_t i=1; i<tag.size(); i++) {
      wstring key = cat+L"#"+util::int2wstring(i); // field number (e.g N#2 -> 2nd field of a "N" tag)
      wstring code = freeling::util::uppercase(tag.substr(i,1));  // feature code
      map<wstring,wstring>::const_iterator f=feat.find(key); // feature name

      if (f==feat.end()) return res;  // position description not found. There is no more information about this tag.
    
      wstring featname = f->second;
      if (code!=L"0" and code!=L"-" and code!=L".") {
        // retrieve values for field+code (e.g. N#2#M -> "M" code found in 2nd field of "N" tag is translated, e.g, as "masc")
        map<wstring,wstring>::const_iterator v = val.find(key+L"#"+code);  
        if (v==val.end()) {
          // no translation found. Invalid PoS tag.
          WARNING(L"Tag "+tag+L": Invalid code '"+code+L"' for feature '"+featname+L"'");
        }
        else {
          // Translation found. Output it if not "0" (0 -> "ignore")
          if (v->second!=L"0")
            res.push_back(make_pair(featname,v->second));
        }
      }
    }

    return res;
  }


  //////////////////////////////////////////////////////////////////////
  /// get EAGLES tag from morphological info given as list <feature,value> pairs 
  ////////////////////////////////////////////////////////////////////

  wstring tagset::msd_to_tag(const wstring &cat,const list<pair<wstring,wstring> > &msd) const {

    wstring s_msd = util::pairlist2wstring(msd,PAIR_SEP,MSD_SEP);
    TRACE(6,L"Converting msd to tag: "+cat+L" "+s_msd);

    // see if the set of features matches a direct translation entry
    map<set<wstring>,wstring>::const_iterator k=direct_inv.find(util::wstring2set(s_msd,MSD_SEP));
    if (k!=direct_inv.end()) return k->second;

    // otherwise, built tag from MSD features.

    /// store feature-value pair in a map, for faster access.
    map<wstring,wstring> fs;
    for (list<pair<wstring,wstring> >::const_iterator p=msd.begin(); p!=msd.end(); p++) {
      fs.insert(make_pair(p->first,p->second));
    }

    /// Get the code for the main category
    wstring tag;
    // if feature pos=xxx is in the msd, use it
    map<wstring,wstring>::iterator n=fs.find(L"pos");
    if (n!=fs.end()) {
      map<wstring,wstring>::const_iterator p = name_inv.find(n->second);
      if (p!=name_inv.end()) {
        tag=p->second;
      }
      else {
        WARNING(L"Invalid pos value in msd: "+util::pairlist2wstring(msd,PAIR_SEP,MSD_SEP));
        tag = cat;
      }
    }
    else 
      // feature pos=xxx was not in the msd, use given category.
      tag = cat;

    // check all features in msd make sense for this category
    for (map<wstring,wstring>::iterator f=fs.begin(); f!=fs.end(); f++) {
      if (f->first!=L"pos" and feat.find(tag+L"#"+f->first)==feat.end()) 
        WARNING(L"Unknown feature '"+f->first+L"' for category '"+tag+L"' when converting msd '"+s_msd+L"'");
    }
    // for each expected position for this category, translate value
    wstring key=tag;
    int i=1;
    map<wstring,wstring>::const_iterator d = feat.find(key+L"#"+util::int2wstring(i));
    while (d!=feat.end()) {
      // compute code for current position of the pos tag
      wstring code = L"0";

      // d->second is the name of the feature in position i of the pos tag.
      // Look for it in given msd      
      map<wstring,wstring>::iterator v = fs.find(d->second);
      if (v!=fs.end()) {
        // v->second is the value of the feature. Translate it using val_inv
        map<wstring,wstring>::const_iterator c = val_inv.find(key+L"#"+util::int2wstring(i)+L"#"+v->second);
        if (c!=val_inv.end())
          code = c->second;
        else 
          WARNING(L"Ignoring unknown value "+v->second+L" for feature "+v->first+L" when converting "+key+L":"+s_msd);
      }
      
      // extend tag with right code
      tag = tag + code;
      // move to next position in the tag, if any
      i++;
      d = feat.find(key+L"#"+util::int2wstring(i));
    }

    TRACE(6,L"Converted: "+tag);
    return tag;
  }


  //////////////////////////////////////////////////////////////////////
  /// get EAGLES tag from morphological info given as string 
  //////////////////////////////////////////////////////////////////////

  wstring tagset::msd_to_tag(const wstring &cat,const wstring &msd) const {
    wstring tag = msd_to_tag (cat,util::wstring2pairlist(msd,PAIR_SEP,MSD_SEP));
    return tag;
  }

} // namespace
