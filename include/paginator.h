#pragma once

#include <iostream>
#include <string>
#include <vector>

template <typename Iterator>
class IteratorRange {
  private:
    Iterator first_, last_;
    std::size_t size_;
  public:
    explicit IteratorRange(Iterator begin, Iterator end)
        : first_(begin), last_(end), size_(static_cast<size_t>(distance(first_, last_))) {}
    Iterator begin() const {
        return first_;
    }
    Iterator end() const {
        return last_;
    }
    size_t size () const {
        return size_;
    }
};

template <typename Iterator>
class Paginator {
private:
    std::vector<IteratorRange<Iterator>> pages_;
public:
    explicit Paginator(Iterator begin, Iterator end, size_t page_size);
    auto begin() const {
        return pages_.begin();
    }
    auto end() const {
        return pages_.end();
    }
    size_t size() const {
        return pages_.size();
    }
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template <typename Iterator>
std::ostream& operator << (std::ostream& os, const IteratorRange<Iterator> &rhs) {
    for (auto it = rhs.begin(); it < rhs.end(); ++it) {
        os << *it;
    }
    return os;
}

template <typename Iterator>
Paginator<Iterator>::Paginator(Iterator begin, Iterator end, size_t page_size) {
    using namespace std;
    if (page_size == 0) {
        throw out_of_range("Zero page size");
    }
    if (begin == end) {
        return;
    }
    while (distance(begin, end) > static_cast<int>(page_size)) {
        auto temp = begin;
        advance(temp, page_size);
        pages_.push_back(IteratorRange(begin, temp));
        begin = temp;
    }
    pages_.push_back(IteratorRange(begin, end));
}
