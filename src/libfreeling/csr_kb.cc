
#include <fstream>
#include <sstream>
#include <cmath>
using std::fabs;

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/csr_kb.h"

#define MOD_TRACENAME L"CSR_KB"
#define MOD_TRACECODE WSD_TRACE

using namespace std;

namespace freeling {
      
  /// ----------------------------------------------------
  /// Class KB functions
    
  ///////////////////////////////////////////////////////
  /// Constructor: create kb, loading data from given file
  ///////////////////////////////////////////////////////
  
  csr_kb::csr_kb(const std::wstring &kbFile, int nit, double thr, double damp) : MaxIterations(nit),
                                                                               Threshold(thr),
                                                                               Damping(damp) {
    wifstream fabr;
    util::open_utf8_file(fabr, kbFile);
    if (fabr.fail()) ERROR_CRASH(L"Error opening file "+kbFile);

    wstring line,syn1,syn2;
    size_t pos1, pos2;
    list<pair<size_t,size_t> > rels;

    num_vertices=0;

    while (getline(fabr,line)) {
      wistringstream sin;
      sin.str(line);
      sin>>syn1>>syn2;

      // add relation vertices (if new). Get position in vector
      pos1 = add_vertex(syn1);
      if (syn2!=L"-") { 
        // not a singleton, add second synset and relations
        pos2 = add_vertex(syn2);

        rels.push_back(make_pair(pos1,pos2));
        rels.push_back(make_pair(pos2,pos1));
      }
    }

    // build CSR from list of relations
    fill_CSR_tables(num_vertices,rels);
  }


  ///////////////////////////////////////////////////////
  /// Fill up graph CSR representation from given list of edges
  ///////////////////////////////////////////////////////

  void csr_kb::fill_CSR_tables(size_t nv, list<pair<size_t,size_t> > &rels) {

    // sort list of edges by source
    rels.sort(util::ascending_first<size_t,size_t>);
    // allocate space for graph tables
    edges.reserve(rels.size());
    first_edge.reserve(nv); num_edges.reserve(nv); out_coef.reserve(nv);

    // fill CSR tables
    size_t n=0;
    size_t r=0;
    list<pair<size_t,size_t> >::const_iterator p=rels.begin();
    while (p!=rels.end() and n<nv) {
      // new node, set first edge to current r
      first_edge[n] = r; 
      // add all relations for this node
      while (p!=rels.end() and p->first==n) {
        edges[r++] = p->second;
        p++;
      }
      num_edges[n] = r - first_edge[n];
      out_coef[n] = 1/static_cast<double>(num_edges[n]);
      n++;
    }
  }

  ///////////////////////////////////////////////////////
  /// Auxiliar const value to be returned when an 
  /// non-existing vertex id is requested.
  ///////////////////////////////////////////////////////
  
  const size_t csr_kb::VERTEX_NOT_FOUND=-1;
  
  ///////////////////////////////////////////////////////
  /// Auxiliary for constructor: add a synset to vertices list, and return  
  /// position in vector where it is added (or where found if already there)
  ///////////////////////////////////////////////////////
  
  size_t csr_kb::add_vertex(const wstring &s) {
    pair<map<wstring,size_t>::iterator,bool> inserted;
    inserted = vertex_index.insert(make_pair(s,num_vertices));
    if (inserted.second) num_vertices++;
    return inserted.first->second;
  }  
  
  ///////////////////////////////////////////////////////
  /// Size of the graph (number of vertices)
  ///////////////////////////////////////////////////////

  size_t csr_kb::size() const { return num_vertices; }
  
  ///////////////////////////////////////////////////////
  /// get vertex index given its id (or VERTEX_NOT_FOUND if not there)
  ///////////////////////////////////////////////////////
  
  size_t csr_kb::get_vertex(const std::wstring &vid) const {
    std::map<std::wstring,size_t>::const_iterator p = vertex_index.find(vid); 
    if (p==vertex_index.end()) return VERTEX_NOT_FOUND;
    else return p->second;      
  }
  

  ////////////////////////////////////////////////////////////////
  /// Use graph to rank given weigthed vertices. 
  /// Ranks are returned in the same map.
  ///////////////////////////////////////////////////////
  
  void csr_kb::pagerank(vector<double> &pv) const {
    // create 2 tmp vectors for ranks, to alternate at each iteration
    vector<double> ranks[2];
    int CURRENT=0;
    int NEXT=1;  
    double initval=1.0/static_cast<double>(num_vertices);
    vector<double>(num_vertices, initval).swap(ranks[CURRENT]);
    vector<double>(num_vertices, 0.0).swap(ranks[NEXT]);

    // --- MAIN LOOP -- apply page rank
    int nit=0;
    double change = Threshold; // make sure it will enter the loop the first time    
    while (nit<MaxIterations and change>=Threshold) {
      change = 0;      

      // for each vertex in the graph, update rank value      
      for (size_t v=0; v<num_vertices; v++) {
        // compute node rank, adding contributions from each incoming edge
        double rank=0.0;
        for (size_t e=first_edge[v]; e < first_edge[v]+num_edges[v]; e++) {
          // Get edge source
          size_t u = edges[e];
          // add influence from u
          rank += ranks[CURRENT][u] * out_coef[u];
        }
        
        // compute NEXT value for rank of current node
        ranks[NEXT][v] = rank*Damping + pv[v]*(1-Damping);
        // add variation to global amount of change
        change += fabs(ranks[NEXT][v] - ranks[CURRENT][v]);
      }
      
      // swap next & current rank vectors for next iteration
      std::swap(NEXT,CURRENT);
      nit++;
    }
    
    // results are in ranks[CURRENT], copy to pv
    pv.swap(ranks[CURRENT]);
  }
  
} // namespace freeling
