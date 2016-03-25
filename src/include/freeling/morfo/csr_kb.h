
#ifndef _CSR_KB_H
#define _CSR_KB_H

#include <map>
#include <list>
#include <vector>
#include <string>

namespace freeling {
  
  /// class kb: Store information associated to a knowledge base 

  class csr_kb {

  public:  
    /// create from file + pagerank params
    csr_kb(const std::wstring &, int, double, double);      
    /// get vertex index given its id (or VERTEX_NOT_FOUND if not there) 
    size_t get_vertex(const std::wstring &) const;

    /// use graph to rank given weigthed vertices. Ranks are returned in the same vector
    void pagerank(std::vector<double> &) const;

    /// get number of vertices of the graph
    size_t size() const;

    /// const value for failed id searches 
    static const size_t VERTEX_NOT_FOUND;
      
  private:
    /// pagerank: maximum number of iterations to perform
    int MaxIterations;
    /// pagerank: threshold to detect that changes are residual
    double Threshold;
    /// pagerank: Damping factor
    double Damping;

    /// index to access vertices by name
    std::map<std::wstring,size_t> vertex_index;
    /// output coefficent for each vertex (1/num_edges)
    std::vector<double> out_coef;

    /// CSR: position in edge table where first edge for this vertex is found
    std::vector<size_t> first_edge;
    /// CSR: num of edges for this vertex
    std::vector<size_t> num_edges;
    /// CSR: edge table, containing edge targets
    std::vector<size_t> edges;
    /// graph size
    size_t num_vertices;

    /// helper to load grap
    size_t add_vertex(const std::wstring &);
    /// helper to load grap
    void fill_CSR_tables(size_t, std::list<std::pair<size_t,size_t> > &);
  };
  
  
} // namespace freeling

#endif
