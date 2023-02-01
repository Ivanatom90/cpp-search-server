#include "document.h"

//using namespace std::literals;
//using namespace literals;
using namespace std;

std::ostream& operator<<(std::ostream& out, const Document& document) { 
    out << "{ " << "document_id = " << document.id << ", " << "relevance = " << document.relevance << ", " << "rating = " << document.rating << " }"; 
    return out; 
} 
