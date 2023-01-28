#pragma once
#include <string>
#include <iostream>
#include <ostream>
#include <istream>

#include "paginator.h"
#include "document.h"

std::string ReadLine();

int ReadLineWithNumber();


template <typename Container>
 auto Paginate  (const Container& c, int page_size) {
    return Paginator(c.begin(), c.end(), page_size);
}

template <typename Iterator>
ostream& operator<<(ostream& out, const IteratorRange<Iterator> iterator_type) {
    for (auto it = iterator_type.begin(); it != iterator_type.end(); it++){
        Document doc = *it;
        out<<"{ document_id = "<<doc.id<<", relevance = "<<doc.relevance<<", rating = "<<doc.rating<<" }";
}
    return out;
}
