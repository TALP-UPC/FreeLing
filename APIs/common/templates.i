////////////////////////////////////////////////////////////////
//
//  templates.i
//  This is a SWIG input file, used to generate java/perl/python APIs.
//
////////////////////////////////////////////////////////////////

%include std_list.i
%include std_vector.i
%include std_map.i
%include std_pair.i

%template(VectorWord) std::vector<freeling::word>;
%template(ListWord) std::list<freeling::word>;
%template(ListAnalysis) std::list<freeling::analysis>;
%template(ListAlternative) std::list<freeling::alternative>;
%template(ListSentence) std::list<freeling::sentence>;
%template(ListParagraph) std::list<freeling::paragraph>;

%template(VectorArgument) std::vector<freeling::argument>;
%template(VectorPredicate) std::vector<freeling::predicate>;

%template(VectorSGMention) std::vector<freeling::semgraph::SG_mention>;
%template(VectorSGArgument) std::vector<freeling::semgraph::SG_argument>;
%template(VectorSGEntity) std::vector<freeling::semgraph::SG_entity>;
%template(VectorSGFrame) std::vector<freeling::semgraph::SG_frame>;

#ifdef FL_API_JAVA
%template(ListParagraphIterator) ListIterator<freeling::paragraph>;
%template(ListSentenceIterator) ListIterator<freeling::sentence>;
%template(ListWordIterator)     ListIterator<freeling::word>;
#endif

%template(ListString) std::list<std::wstring>;
%template(ListInt) std::list<int>;
%template(VectorListInt) std::vector<std::list<int> >;
%template(VectorListString) std::vector<std::list<std::wstring> >;
%template(VectorString) std::vector<std::wstring>;

%template(PairDoubleString) std::pair<double,std::wstring >;
%template(VectorPairDoubleString) std::vector<std::pair<double,std::wstring> >;

%template(PairStringString) std::pair<std::wstring,std::wstring >;
%template(VectorPairStringString) std::vector<std::pair<std::wstring,std::wstring> >;

%template(PairStringInt) std::pair<std::wstring,int>;
%template(PairStringDouble) std::pair<std::wstring,double>;
%template(ListPairStringDouble) std::list<std::pair<std::wstring,double> >;
