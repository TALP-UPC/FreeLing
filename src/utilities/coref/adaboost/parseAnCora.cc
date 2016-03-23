//////////////////////////////////////////////////////////////////
//    parseAncora - Parses CESS and AnCora
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <iconv.h>
#include <errno.h>

#include <list>
#include <fstream>
#include <iostream>

using namespace std;

#include "freeling.h"
#include "freeling/traces.h"

#ifndef LIBXML_TREE_ENABLED
  #error "Tree support not compiled in\n"
#else

#define TYPE_LIMITED		1
#define TYPE_PROPORTIONAL	2

#define TYPE_WORD			0
#define TYPE_DE				1

struct SENT {
  string id;
  list<struct DE *> de;
  string texto;
  list<string> text;
  list<string> tags;
};
struct DE {
  int type;	//0 - W, 1 - DE
  int numDe; //Only to check the limit of negative samples.
  int parentId;
  string id;
  int pos;
  int sent;
  //	string type1;
  string type2;
  //	string type3;
  string corefLink;
  string falseCorefLink;
  //	string corefLink_type;
  list<struct DE *> de;
  //	string word;
  list<string> text;
  list<string> tags;
};
struct SAMPLE_DES {
  struct DE *de1;
  struct DE *de2;
};

list<struct SENT *> article;
list<struct DE *> allDEs;
list<list<struct DE *> *> groupsDEs;
list<struct SAMPLE_DES *> negativeDEs;
list<struct SAMPLE_DES *> positiveDEs;

ifstream *myfile;
ofstream *outfile;
int globalPos;

//UTF8
const char * EUCSET = "UTF8";
const char * OUTSET = "iso88591";

/////////////////////////////////////////////////////////////////////////////
/// Given a character string in UTF8, convert it into iso8859-1 and
/// return the result in allocated memory.
/////////////////////////////////////////////////////////////////////////////

char *utf82iso8859(char * euc){
  unsigned start_len;
  static iconv_t utf82iso8859t;
  int init = 0;
  size_t iconv_value;
  char * utf8, * utf8start;
  unsigned len, utf8len, utf8lenstart;
  int v;
  
  if (init == 0){
    init = 1;
    utf82iso8859t = iconv_open (OUTSET, EUCSET);
    if (utf82iso8859t == (iconv_t) -1){
      return NULL;
    }
  }
  start_len = len  = strlen (euc);
  
  utf8lenstart = utf8len = (3*len)/2 + 1;
  utf8start =   utf8 = (char *) calloc (utf8len, 1);
  
  iconv_value = iconv (utf82iso8859t, & euc, & len, & utf8, & utf8len);
  v = iconv_close (utf82iso8859t);
  
  return utf8start;
}

/////////////////////////////////////////////////////////////////////////////
/// Strips _ and spaces in the string.
/////////////////////////////////////////////////////////////////////////////

static void strip(unsigned char *buf){
  unsigned int i, j, l;
  
  l = strlen((char *)buf);
  i = 0;
  j = 0;
  while(i<l){
    while(buf[i] == 194 || buf[i] == 131){
      i++;
    }
    if(isspace(buf[i])){
      i++;
    } else {
      if(buf[i] == '_')
	buf[i] = ' ';
      if(i != j)
	buf[j] = buf[i];
      i++;
      j++;
    }
  }
  buf[j] = 0;
}

/////////////////////////////////////////////////////////////////////////////
/// Parse a Word and returns a DE struct of type Word
/////////////////////////////////////////////////////////////////////////////

static struct DE * parseWord(xmlNode * a_node){
  xmlNode *cur_node = NULL;
  
  struct DE *de = new DE;
  
  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
#ifdef DEBUG
      cout << "Unk word element: " << (char *)cur_node->name << endl;
#endif
    } else if (cur_node->type == XML_TEXT_NODE) {
      strip((unsigned char *)cur_node->content);
      if(strlen((char *)cur_node->content) > 0){
	
	//Reads CESS file
	char buffer[1000];
	string str1, tag, w2;
	int p1, p2, p3, p4;
	if (myfile->good()){
	  if(!myfile->eof()){
	    myfile->getline(buffer, 1000);
	    str1 = buffer;
	    while((p2 = str1.find(")")) == -1 && !myfile->eof()){
	      myfile->getline(buffer, 1000);
	      str1 = buffer;
	    }
	    if((p2 = str1.find(")")) != -1){
	      p1 = str1.rfind("(")+1;
	      
	      str1 = str1.substr(p1, p2-p1);
	      p3 = str1.find(" ");
	      p4 = str1.rfind(" ");
	      tag = str1.substr(0, p3);
	      //w2 = str1.substr(p4+1, str1.size()-1-p4).c_str(); // LEMA
	      w2 = str1.substr(p3+1, p4-p3-1).c_str(); //WORD
	    } else { 
	      cout << "ERROR: Palabra no encontrada" << endl;
	    }
	  } else { 
	    cout << "ERROR: Fin de fichero" << endl;
	  }
	} else {
	  cout << "ERROR: En el fichero" << endl;
	}
	//End read CESS file
	
	char *tmp = (char *)cur_node->content;
	char *tmp2 = utf82iso8859 (tmp);
	string w = tmp2;
	free(tmp2);
	//string w = (char *)buff;
	globalPos++;
	de->pos = globalPos;
	de->type = TYPE_WORD;
	de->text.push_back(w2);
	de->tags.push_back(tag);
      }
    }
  }
  return(de);
}

/////////////////////////////////////////////////////////////////////////////
/// Parse a DE and returns a DE struct of type DE
/////////////////////////////////////////////////////////////////////////////

static struct DE * parseDe(xmlNode * a_node){
  xmlNode *cur_node = NULL;
  xmlAttr *cur_attr = NULL;
  
  struct DE *de = new DE;
  de->type = TYPE_DE;
  de->pos = globalPos;
  
  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      if(strcmp((char *)cur_node->name, "de") == 0){
	de->de.push_back(parseDe(cur_node->children));
      } else if(strcmp((char *)cur_node->name, "w") == 0){
	de->de.push_back(parseWord(cur_node->children));
      } else if(strcmp((char *)cur_node->name, "seg") == 0){
	de->de.push_back(parseDe(cur_node->children));
      } else if(strcmp((char *)cur_node->name, "clit") == 0){
	de->de.push_back(parseDe(cur_node->children));
      } else if(strcmp((char *)cur_node->name, "corefLink") == 0){
      } else if(strcmp((char *)cur_node->name, "semtypeLink") == 0){
      } else if(strcmp((char *)cur_node->name, "falseposLink") == 0){
      } else {
#ifdef DEBUG
	cout << "Unk DE element: " << (char *)cur_node->name << endl;
#endif
      }
      cur_attr = cur_node->properties;
      while(cur_attr){
	if(strcmp((char *)cur_node->name, "de") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	  de->de.back()->id = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "de") == 0 && strcmp((char *)cur_attr->name, "type2") == 0){
	  de->de.back()->type2 = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "corefLink") == 0 && strcmp((char *)cur_attr->name, "anchor") == 0){
	  de->corefLink = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "falseposLink") == 0 && strcmp((char *)cur_attr->name, "anchor") == 0){
	  de->falseCorefLink = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "clit") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	  de->de.back()->id = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "clit") == 0 && strcmp((char *)cur_attr->name, "type2") == 0){
	  de->de.back()->type2 = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "de") == 0 && strcmp((char *)cur_attr->name, "type1") == 0){
	} else if(strcmp((char *)cur_node->name, "de") == 0 && strcmp((char *)cur_attr->name, "type3") == 0){
	} else if(strcmp((char *)cur_node->name, "corefLink") == 0 && strcmp((char *)cur_attr->name, "type") == 0){
	} else if(strcmp((char *)cur_node->name, "corefLink") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	} else if(strcmp((char *)cur_node->name, "seg") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	} else if(strcmp((char *)cur_node->name, "corefLink") == 0 && strcmp((char *)cur_attr->name, "subtype") == 0){
	} else if(strcmp((char *)cur_node->name, "falseposLink") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	} else if(strcmp((char *)cur_node->name, "semtypeLink") == 0 && strcmp((char *)cur_attr->name, "anchor") == 0){
	} else if(strcmp((char *)cur_node->name, "semtypeLink") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	} else if(strcmp((char *)cur_node->name, "clit") == 0 && strcmp((char *)cur_attr->name, "type1") == 0){
	} else {
#ifdef DEBUG
	  printf("Unk DE attribute: 	attr node: %s\n", cur_node->name);
	  printf("Unk DE attribute: 	attr name: %s\n", cur_attr->name);
	  printf("Unk DE attribute: 	attr val: %s\n", cur_attr->children->content);
#endif
	}
	cur_attr = cur_attr->next;
      }
    }
  }
  return(de);
}

/////////////////////////////////////////////////////////////////////////////
/// Parse and returns a Sent
/////////////////////////////////////////////////////////////////////////////

static struct SENT * parseSent(xmlNode * a_node){
  xmlNode *cur_node = NULL;
  xmlAttr *cur_attr = NULL;
  
  struct SENT *sent = new SENT;
  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      if(strcmp((char *)cur_node->name, "de") == 0){
	sent->de.push_back(parseDe(cur_node->children));
      } else if(strcmp((char *)cur_node->name, "w") == 0){
	sent->de.push_back(parseWord(cur_node->children));
      } else if(strcmp((char *)cur_node->name, "seg") == 0){
	sent->de.push_back(parseDe(cur_node->children));
      } else if(strcmp((char *)cur_node->name, "clit") == 0){
	sent->de.push_back(parseDe(cur_node->children));
      } else {
#ifdef DEBUG
	cout << "Unk sent element: " << (char *)cur_node->name << endl;
#endif
      }
      cur_attr = cur_node->properties;
      while(cur_attr){
	if(strcmp((char *)cur_node->name, "de") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	  sent->de.back()->id = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "de") == 0 && strcmp((char *)cur_attr->name, "type2") == 0){
	  sent->de.back()->type2 = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "seg") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	  sent->de.back()->id = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "clit") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	  sent->de.back()->id = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "clit") == 0 && strcmp((char *)cur_attr->name, "type2") == 0){
	  sent->de.back()->type2 = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "de") == 0 && strcmp((char *)cur_attr->name, "type1") == 0){
	} else if(strcmp((char *)cur_node->name, "de") == 0 && strcmp((char *)cur_attr->name, "type3") == 0){
	} else if(strcmp((char *)cur_node->name, "clit") == 0 && strcmp((char *)cur_attr->name, "type1") == 0){
	} else {
#ifdef DEBUG
	  printf("Unk sent attribute: 	attr node: %s\n", cur_node->name);
	  printf("Unk sent attribute: 	attr name: %s\n", cur_attr->name);
	  printf("Unk sent attribute: 	attr val: %s\n", cur_attr->children->content);
#endif
	}
	cur_attr = cur_attr->next;
      }
    }
  }
  return(sent);
}

/////////////////////////////////////////////////////////////////////////////
/// Parse an article and fills article list
/////////////////////////////////////////////////////////////////////////////

static void parseArticle(xmlNode * a_node){
  xmlNode *cur_node = NULL;
  xmlAttr *cur_attr = NULL;
  
  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      if(strcmp((char *)cur_node->name, "article") == 0){
	parseArticle(cur_node->children);
      } else if(strcmp((char *)cur_node->name, "sent") == 0){
	article.push_back(parseSent(cur_node->children));
      } else {
#ifdef DEBUG
	cout << "Unk article element: " << (char *)cur_node->name << endl;
#endif
      }
      cur_attr = cur_node->properties;
      while(cur_attr){
	if(strcmp((char *)cur_node->name, "sent") == 0 && strcmp((char *)cur_attr->name, "id") == 0){
	  article.back()->id = (char *)cur_attr->children->content;
	} else if(strcmp((char *)cur_node->name, "article") == 0 && strcmp((char *)cur_attr->name, "file") == 0){
	} else if(strcmp((char *)cur_node->name, "article") == 0 && strcmp((char *)cur_attr->name, "nfrases") == 0){
	} else if(strcmp((char *)cur_node->name, "article") == 0 && strcmp((char *)cur_attr->name, "output") == 0){
	} else if(strcmp((char *)cur_node->name, "sent") == 0 && strcmp((char *)cur_attr->name, "file") == 0){
	} else if(strcmp((char *)cur_node->name, "sent") == 0 && strcmp((char *)cur_attr->name, "src") == 0){
	} else {
#ifdef DEBUG
	  printf("Unk article Attribute: 	attr name: %s\n", cur_attr->name);
	  printf("Unk article Attribute: 	attr val: %s\n", cur_attr->children->content);
#endif
	}
	cur_attr = cur_attr->next;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
/// Add a DE to his group of coreferents or creates a new group
/////////////////////////////////////////////////////////////////////////////

static void addToGroup(struct DE *de){
  list<list<struct DE *> *>::iterator grpIt;
  list<struct DE *>::iterator deIt;
  bool found = false;
  
  for(grpIt = groupsDEs.begin(); grpIt != groupsDEs.end() && !found; ++grpIt){
    for(deIt = (*grpIt)->begin(); deIt != (*grpIt)->end() && !found; ++deIt){
      if(de->corefLink == (*deIt)->id && de->corefLink != ""){
	found = true;
	(*grpIt)->push_back(de);
      } else if(de->id == (*deIt)->corefLink && de->id != ""){
	found = true;
	(*grpIt)->push_back(de);
      }
    }
  }
  if(!found){
    list<struct DE *> *l = new list<struct DE *>;
    l->push_back(de);
    groupsDEs.push_back(l);
  }
}

#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
/// Prints a DE only to debug purposes. Needs to declare DEBUG constant
/////////////////////////////////////////////////////////////////////////////

static void printDe(struct DE *de){
  list<struct DE *>::iterator deIt;
  list<string>::iterator textIt, tagIt;
  
  if(de->id != ""){
    cout << "De: " << de->id << "	";
    cout << "Pos: " << de->pos << "-" << (de->pos + de->text.size()-1) <<  "	";
    cout << "corefLink: " << de->corefLink << "	";
    cout << "falseCorefLink: " << de->falseCorefLink << "	";
    cout << "type2: " << de->type2 << "	";
    cout << "W: ";
    textIt = de->text.begin();
    tagIt = de->tags.begin();
    while(textIt != de->text.end()){
      cout << *textIt << "/" << *tagIt << " ";
      ++textIt;
      ++tagIt;
    }
    cout << endl;
  }
}
#endif

/////////////////////////////////////////////////////////////////////////////
/// Propagate a DE recursively and fills the tags and text (all or onlyhead)
/// Converts the NE classification from the AnCora to the format of Freeling (EAGLES v2.0)
/////////////////////////////////////////////////////////////////////////////

static void propagateDe(bool onlyhead, struct DE *de, unsigned int *numDe, unsigned int numSent){
  bool stop = false;
  list<struct DE *>::iterator deIt;
  list<string>::iterator textIt, tagIt;
  
  for(deIt = de->de.begin(); deIt != de->de.end(); ++deIt){
    (*deIt)->sent = numSent;
    (*deIt)->numDe = *numDe;
    (*deIt)->parentId = de->numDe;
    (*numDe)++;
    allDEs.push_back(*deIt);
    addToGroup(*deIt);
    propagateDe(onlyhead, *deIt, numDe, numSent);
    textIt = (*deIt)->text.begin();
    tagIt = (*deIt)->tags.begin();
    if(onlyhead && (*deIt)->type != TYPE_WORD && de->text.size() > 0){
      stop = true;
    }
    
    while(textIt != (*deIt)->text.end() && !stop){
      if((*tagIt).compare(0, 2, "np") == 0){
	if((*deIt)->type2 == "NE-loc" || de->type2 == "NE-loc"){
	  (*tagIt)[4] = 'g';
	  (*tagIt)[5] = '0';
	} else if((*deIt)->type2 == "NE-pers" || de->type2 == "NE-pers"){
	  (*tagIt)[4] = 's';
	  (*tagIt)[5] = 'p';
	} else if((*deIt)->type2 == "NE-num" || de->type2 == "NE-num"){
	} else if((*deIt)->type2 == "NE-org" || de->type2 == "NE-org"){
	  (*tagIt)[4] = 'o';
	  (*tagIt)[5] = '0';
	} else if((*deIt)->type2 == "NE-other" || de->type2 == "NE-other"){
	  (*tagIt)[4] = 'v';
	  (*tagIt)[5] = '0';
	} else if((*deIt)->type2 == "NE-date" || de->type2 == "NE-date"){
	} else if((*deIt)->type2 == "nne" || de->type2 == "nne"){
	} else if((*deIt)->type2 == "spec" || de->type2 == "spec"){
	} else if((*deIt)->type2 != "" || de->type2 != ""){
#ifdef DEBUG
	  cout << "Error NE: " << (*deIt)->type2 << " " << de->type2 << endl;
#endif
	}
      }
      de->text.push_back(*textIt);
      de->tags.push_back(*tagIt);
      ++textIt;
      ++tagIt;
    }
#ifdef DEBUG
    printDe(*deIt);
#endif
  }
}

/////////////////////////////////////////////////////////////////////////////
/// Plains all the DE calling propagateDe
/// Converts the NE classification from the AnCora to the format of Freeling (EAGLES v2.0)
/////////////////////////////////////////////////////////////////////////////

static void plain_des(bool onlyhead){
  bool stop = false;
  list<struct SENT *>::iterator sentIt;
  list<struct DE *>::iterator deIt;
  list<string>::iterator textIt, tagIt;
  unsigned int numSent = 0, numDe = 0;
  
  for(sentIt = article.begin(); sentIt != article.end(); ++sentIt){
    (*sentIt)->texto = "";
    for(deIt = (*sentIt)->de.begin(); deIt != (*sentIt)->de.end(); ++deIt){
      (*deIt)->sent = numSent;
      (*deIt)->numDe = numDe;
      (*deIt)->parentId = -1;
      numDe++;
      allDEs.push_back(*deIt);
      addToGroup(*deIt);
      propagateDe(onlyhead, *deIt, &numDe, numSent);
      
      for(textIt = (*deIt)->text.begin(); textIt != (*deIt)->text.end(); ++textIt){
	(*sentIt)->texto += " " + (*textIt);
      }
      textIt = (*deIt)->text.begin();
      tagIt = (*deIt)->tags.begin();
      if(onlyhead && (*deIt)->type != TYPE_WORD && (*sentIt)->text.size() > 0){
	stop = true;
      }
      while(textIt != (*deIt)->text.end() && !stop){
	
	if((*tagIt).compare(0, 2, "np") == 0){
	  if((*deIt)->type2 == "NE-loc"){
	    (*tagIt)[4] = 'g';
	    (*tagIt)[5] = '0';
	  } else if((*deIt)->type2 == "NE-pers"){
	    (*tagIt)[4] = 's';
	    (*tagIt)[5] = 'p';
	  } else if((*deIt)->type2 == "NE-num"){
	  } else if((*deIt)->type2 == "NE-org"){
	    (*tagIt)[4] = 'o';
	    (*tagIt)[5] = '0';
	  } else if((*deIt)->type2 == "NE-other"){
	    (*tagIt)[4] = 'v';
	    (*tagIt)[5] = '0';
	  } else if((*deIt)->type2 == "NE-date"){
	  } else if((*deIt)->type2 == "nne"){
	  } else if((*deIt)->type2 == "spec"){
	  } else if((*deIt)->type2 != ""){
#ifdef DEBUG
	    cerr << (*deIt)->type2 << endl;
#endif
	  }
	}
	(*sentIt)->texto += " " + (*textIt);
	(*sentIt)->text.push_back(*textIt);
	(*sentIt)->tags.push_back(*tagIt);
	++textIt;
	++tagIt;
      }
#ifdef DEBUG
      printDe(*deIt);
#endif
    }
#ifdef DEBUG
    cout << "SENT: "<< (*sentIt)->texto << endl;
#endif
    numSent++;
  }
}

/////////////////////////////////////////////////////////////////////////////
/// Outputs a negative sample from two DE's
/////////////////////////////////////////////////////////////////////////////

static void print_negative(struct DE * de1, struct DE * de2){
  list<struct DE *>::iterator deIt;
  list<string>::iterator textIt, tagIt;
  
  (*outfile) << "-" << endl;
  (*outfile) << de1->sent << endl;
  (*outfile) << de1->pos << endl;
  (*outfile) << (de1->pos + de1->text.size()-1) << endl;
  for(textIt = de1->text.begin(); textIt != de1->text.end(); ++textIt){
    (*outfile) << *textIt << " ";
  }
  (*outfile) << endl;
  for(tagIt = de1->tags.begin(); tagIt != de1->tags.end(); ++tagIt){
    (*outfile) << *tagIt << " ";
  }
  (*outfile) << endl;
  (*outfile) << de2->sent << endl;
  (*outfile) << de2->pos << endl;
  (*outfile) << (de2->pos + de2->text.size()-1) << endl;
  for(textIt = de2->text.begin(); textIt != de2->text.end(); ++textIt){
    (*outfile) << *textIt << " ";
  }
  (*outfile) << endl;
  for(tagIt = de2->tags.begin(); tagIt != de2->tags.end(); ++tagIt){
    (*outfile) << *tagIt << " ";
  }
  (*outfile) << endl;
}

/////////////////////////////////////////////////////////////////////////////
/// Outputs a negative sample from one SAMPLE_DES
/////////////////////////////////////////////////////////////////////////////

static void print_negative(struct SAMPLE_DES *ns){
  struct DE * de1 = ns->de1;
  struct DE * de2 = ns->de2;
  
  print_negative(de1, de2);
}

/////////////////////////////////////////////////////////////////////////////
/// Adds a negative to the list of negative candidates.
/////////////////////////////////////////////////////////////////////////////

static void add_negative(int numMax, struct DE * de1, struct DE * de2){
  list<struct DE *>::iterator deIt;
  list<string>::iterator textIt, tagIt;
  
  deIt = allDEs.begin();
  while(deIt != allDEs.end()){
    if((*deIt)->id == de1->id){
      break;
    }
    ++deIt;
  }
  if(deIt != allDEs.end()){
    ++deIt;
  }
  while(deIt != allDEs.end()){
    if((*deIt)->id == de2->id){
      break;
    }
    if(((de2->numDe - (*deIt)->numDe) <= numMax) || numMax == 0){
      if((*deIt)->id != "" && ((*deIt)->text.front()) != "*0*"){
	//				cout << "	-" << (*deIt)->id << ":" << de2->id << endl;
	struct SAMPLE_DES *ns = new struct SAMPLE_DES;
	ns->de1 = (*deIt);
	ns->de2 = de2;
	negativeDEs.push_back(ns);
      }
    }
    ++deIt;
  }
}

/////////////////////////////////////////////////////////////////////////////
/// Adds a positive to the list of positive candidates.
/////////////////////////////////////////////////////////////////////////////

static void add_positive(struct DE * de1, struct DE * de2){
  struct SAMPLE_DES *ns = new struct SAMPLE_DES;
  ns->de1 = de1;
  ns->de2 = de2;
  positiveDEs.push_back(ns);
  
}

/////////////////////////////////////////////////////////////////////////////
/// Outputs a positive sample from one SAMPLE_DES
/////////////////////////////////////////////////////////////////////////////

static void print_positive(struct SAMPLE_DES *ns){
  list<string>::iterator textIt, tagIt;
  struct DE * de1 = ns->de1;
  struct DE * de2 = ns->de2;
  
  (*outfile) << "+" << endl;
  (*outfile) << de1->sent << endl;
  (*outfile) << de1->pos << endl;
  (*outfile) << (de1->pos + de1->text.size()-1) << endl;
  for(textIt = de1->text.begin(); textIt != de1->text.end(); ++textIt){
    (*outfile) << *textIt << " ";
  }
  (*outfile) << endl;
  for(tagIt = de1->tags.begin(); tagIt != de1->tags.end(); ++tagIt){
    (*outfile) << *tagIt << " ";
  }
  (*outfile) << endl;
  (*outfile) << de2->sent << endl;
  (*outfile) << de2->pos << endl;
  (*outfile) << (de2->pos + de2->text.size()-1) << endl;
  for(textIt = de2->text.begin(); textIt != de2->text.end(); ++textIt){
    (*outfile) << *textIt << " ";
  }
  (*outfile) << endl;
  for(tagIt = de2->tags.begin(); tagIt != de2->tags.end(); ++tagIt){
    (*outfile) << *tagIt << " ";
  }
  (*outfile) << endl;
}

/////////////////////////////////////////////////////////////////////////////
/// Outputs all the positive samples (all the combinations or simple combinations (Like Soon et al.))
/////////////////////////////////////////////////////////////////////////////

static int print_positives(bool allpos){
  list<list<struct DE *> *>::iterator grpIt;
  list<struct DE *>::iterator deIt, deIt2;
  list<string>::iterator textIt, tagIt;
  list <struct SAMPLE_DES *>::iterator itN;
  struct DE *de1, *de2;
  int countp = 0;
  
  for(grpIt = groupsDEs.begin(); grpIt != groupsDEs.end(); ++grpIt){
    if(allpos){
      deIt = (*grpIt)->begin();
      while( deIt != (*grpIt)->end()){
	deIt2 = deIt;
	++deIt2;
	while( deIt2 != (*grpIt)->end()){
	  if(((*deIt)->text.front()) != "*0*" && ((*deIt2)->text.front()) != "*0*"){
	    add_positive((*deIt), (*deIt2));
	    countp++;
	  }
	  ++deIt2;
	}
	++deIt;
      }
    } else {
      deIt = (*grpIt)->begin();
      if(deIt != (*grpIt)->end()){
	de1 = (*deIt);
	++deIt;
      }
      while(deIt != (*grpIt)->end()){
	de2 = (*deIt);
	if((de1->text.front()) != "*0*" && (de2->text.front()) != "*0*"){
	  add_positive(de1, de2);
	  countp++;
	}
	de1 = de2;
	++deIt;
      }
    }
  }
  itN = positiveDEs.begin();
  while(itN != positiveDEs.end()){
    print_positive(*itN);
    ++itN;
  }
  cout << "	Positivos: " << countp << endl;
  return(countp);
}

/////////////////////////////////////////////////////////////////////////////
/// Returns if id1 and id2 exists in the list of positives
/////////////////////////////////////////////////////////////////////////////

static bool is_positive(int id1, int id2){
  list <struct SAMPLE_DES *>::iterator itP;
  
  itP = positiveDEs.begin();
  while(itP != positiveDEs.end()){
    if((*itP)->de1->numDe == id1 &&  (*itP)->de2->numDe == id2){
      return true;
    }
    ++itP;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////
/// Outputs all the negatives (not in the "Soon" way) limited by distance
/////////////////////////////////////////////////////////////////////////////

static void print_allnegatives(int limneg){
  list<struct DE *>::iterator deIt1, deIt2;
  list <struct SAMPLE_DES *>::iterator itN;
  int count, allCount = 0;
  
  deIt1 = allDEs.begin();
  while (deIt1 != allDEs.end()){
    deIt2 = deIt1;
    ++deIt2;
    count = 0;
    if((*deIt1)->type == TYPE_DE){
      while (deIt2 != allDEs.end() && count < limneg){
	if((*deIt2)->type == TYPE_DE){
	  if(!is_positive((*deIt1)->numDe, (*deIt2)->numDe)) {
	    allCount++;
	    print_negative(*deIt1, *deIt2);
	  } else {
	    break;
	  }
	  count++;
	}
	++deIt2;
      }
    }
    ++deIt1;
  }
  
  cout << "	Negativos: " << allCount << endl;
}

/////////////////////////////////////////////////////////////////////////////
/// Filtres and outputs the negative samples
/////////////////////////////////////////////////////////////////////////////

static void print_negatives(int type, int limit, double re, int numpos, bool elimFalsesNegatives){
  list<list<struct DE *> *>::iterator grpIt;
  list<struct DE *>::iterator deIt;
  list<string>::iterator textIt, tagIt;
  struct DE *de1, *de2;
  
  for(grpIt = groupsDEs.begin(); grpIt != groupsDEs.end(); ++grpIt){
    deIt = (*grpIt)->begin();
    if(deIt != (*grpIt)->end()){
      de1 = (*deIt);
      ++deIt;
    }
    while(deIt != (*grpIt)->end()){
      de2 = (*deIt);
      if((de1->text.front()) != "*0*" && (de2->text.front()) != "*0*"){
	add_negative(limit, de1, de2);
      }
      de1 = de2;
      ++deIt;
    }
  }
  
  list <struct SAMPLE_DES *>::iterator itN;
  if(elimFalsesNegatives){
    itN = negativeDEs.begin();
    while(itN != negativeDEs.end()){
      if(is_positive((*itN)->de1->parentId, (*itN)->de2->numDe))
	itN = negativeDEs.erase(itN);
      else
	++itN;
    }
  }
  
  if(type == TYPE_PROPORTIONAL){
    while(negativeDEs.size()/((double)numpos) > re){
      int rn = rand() % negativeDEs.size();
      itN = negativeDEs.begin();
      while(rn > 0){
	rn--;
	++itN;
      }
      negativeDEs.erase(itN);
    }
  }
  
  itN = negativeDEs.begin();
  while(itN != negativeDEs.end()){
    print_negative(*itN);
    ++itN;
  }
  cout << "	Negativos: " << negativeDEs.size() << endl;
}

/////////////////////////////////////////////////////////////////////////////
/// Main process
/////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv){
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  int pos=1, limit=0, limneg=0;
  double re = 1.0;
  bool allpos=false, onlyhead=false;
  int type, numpos;;
  char xmlfile[256], cessfile[256], outputfile[256];
  bool elimFalsesNegatives = false;
  
  if(argc < 6) {
    cout << "parseAncora [-limit n] [-re m] [-allpos] [-onlyhead] AnCoraFile CESSFile OutputFile" << endl;
    cout << "	-elimFN: Eliminates false negatives." << endl;
    cout << "	-allpos: Generates all the conbinations of positive samples." << endl;
    cout << "	-limit n: Is the max separation over negatives samples (0 unlimited)." << endl;
    cout << "	-re m: m (float) is the proportion of negatives over positives (per document)." << endl;
    cout << "	-allneg n: Outputs all the posibles negatives at max n distance." << endl;
    cout << "	AnCoraFile: Is the annotated xml Ancora file." << endl;
    cout << "	CESSFile: Is the corresponding CESS file with AnCoraFile." << endl;
    cout << "	OutputFile: Is the output file for the samples generated." << endl;
    return(1);
  }
  
  type = TYPE_LIMITED;
  while (strncmp(argv[pos], "-", 1) == 0){
    if(strncmp(argv[pos], "-onlyhead", 9) == 0){
      onlyhead = true;
      pos +=1;
    } else if(strncmp(argv[pos], "-elimFN", 7) == 0){
      elimFalsesNegatives = true;
      pos +=1;
    } else if(strncmp(argv[pos], "-allpos", 7) == 0){
      allpos = true;
      pos +=1;
    } else if(strncmp(argv[pos], "-limit", 6) == 0){
      limit = atoi(argv[pos+1]);
      pos +=2;
    } else if(strncmp(argv[pos], "-allneg", 7) == 0){
      limneg = atoi(argv[pos+1]);
      pos +=2;
    } else if(strncmp(argv[pos], "-re", 3) == 0){
      re = atof(argv[pos+1]);
      pos +=2;
      type = TYPE_PROPORTIONAL;
    } else {
      cout << "Error: Invalid param " << argv[pos] << endl;
    }
  }
  strcpy(xmlfile, argv[pos++]);
  strcpy(cessfile, argv[pos++]);
  strcpy(outputfile, argv[pos]);
  
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION
    
    /*parse the file and get the DOM */
    doc = xmlReadFile(xmlfile, "UTF-8", 0);
  
  if (doc == NULL) {
    printf("error: could not parse file %s\n", xmlfile);
  }
  
  /*Get the root element node */
  root_element = xmlDocGetRootElement(doc);
  
  globalPos = 0;
  
  myfile = new ifstream(cessfile, ios::in|ios::binary);
  outfile = new ofstream(outputfile, ios::out|ios::binary|ios::app);
  
  parseArticle(root_element);
  /*free the document */
  xmlFreeDoc(doc);
  
  /*
   *Free the global variables that may
   *have been allocated by the parser.
   */
  xmlCleanupParser();
  
  plain_des(onlyhead);
  
  //	cerr << "File :" << xmlfile << endl;
  numpos = print_positives(allpos);
  if(limneg > 0){
    print_allnegatives(limneg);
  } else {
    print_negatives(type, limit, re, numpos, elimFalsesNegatives);
  }
  
  return 0;
}
#endif
