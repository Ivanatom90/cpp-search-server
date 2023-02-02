#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <cmath>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator range_begin, Iterator range_end)
        : range_begin_(range_begin), range_end_(range_end) {

    }
    Iterator begin() const {
        return range_begin_;
    }

    Iterator end()  const {
        return range_end_;
    }

    Iterator size() const {
        return range_end_ - range_begin_;
    }

private:
    Iterator range_begin_;
    Iterator range_end_;
};




template <typename Iterator>
class Paginator {
    public:
    Paginator(Iterator begin_range, Iterator end_range, size_t page_size){

        auto it = begin_range;
        int iteration = distance(begin_range, end_range)/page_size;
        //for (auto iter=it; iter != end_range; iter+=page_size )
        for (int i = 0; i<iteration; i++)
        {
            page.push_back(IteratorRange<Iterator>(it, it+page_size));
            it+=page_size;
        }


    }

    auto begin() const {
        return page.begin();
    }

    auto end() const {
        return page.end();
    }

    int size() const {
        return  page.size();
    }



std::vector <IteratorRange<Iterator>> page;


};

template <typename Container>
auto  Paginate(const Container& c, size_t page_size) {
    return Paginator(c.begin(), c.end(), page_size);
}

template <typename It>
std::ostream& operator<<(std::ostream& out, const IteratorRange<It>& range) {
    for (It it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}

