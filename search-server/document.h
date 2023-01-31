#pragma once

#include <iostream>
#include <ostream>
#include <paginator.h>

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator> iterator_type) {
    for (auto it = iterator_type.begin(); it != iterator_type.end(); it++){
        Document doc = *it;
        out<<"{ document_id = "<<doc.id<<", relevance = "<<doc.relevance<<", rating = "<<doc.rating<<" }";
}
    return out;
}

