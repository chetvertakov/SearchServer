#pragma once

#include <string>
#include <vector>

#include "search_server.h"

class VectorWrapper {
private:
    class BasicIterator {
        friend class VectorWrapper;
    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = Document;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        BasicIterator() = delete;
        BasicIterator(const std::vector<std::vector<Document>> &c);
        BasicIterator(const std::vector<std::vector<Document>> &c, size_t segment, size_t offset);
        [[nodiscard]] bool operator==(const BasicIterator& rhs) const noexcept;
        [[nodiscard]] bool operator!=(const BasicIterator& rhs) const noexcept;
        [[nodiscard]] Document operator*() const noexcept;
        BasicIterator& operator++() noexcept;
        BasicIterator operator++(int) noexcept;
    private:
        size_t segment_ = 0;
        size_t offset_ = 0;
        size_t size_ = 0;
        const std::vector<std::vector<Document>> &data_;
    };
public:
    using value_type = Document;
    using reference = value_type&;
    using const_reference = const value_type&;
    using Iterator = BasicIterator;

    VectorWrapper(std::vector<std::vector<Document>> &&data);

    [[nodiscard]] Iterator begin() noexcept {
        return Iterator(data_);
    }
    [[nodiscard]] Iterator end() noexcept {
        return Iterator(data_, data_.size() - 1, data_.at(data_.size() - 1).size() );
    }
    size_t size() const noexcept {
        return data_.size();
    }
private:
    std::vector<std::vector<Document>> data_;
};

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);

VectorWrapper ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);
