#include <execution>

#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer &search_server,
                                                    const std::vector<std::string> &queries) {
    std::vector<std::vector<Document>> result(queries.size());

    std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(),
                   [&search_server](auto &query) {
        return search_server.FindTopDocuments(query);
    });
    return result;
}

VectorWrapper ProcessQueriesJoined(const SearchServer &search_server,
                                           const std::vector<std::string> &queries) {
    return VectorWrapper(ProcessQueries(search_server, queries));
}


VectorWrapper::BasicIterator::BasicIterator(const std::vector<std::vector<Document>> &c) : data_(c) {
    size_ = c.size();
    segment_ = 0;
    offset_ = 0;
}

VectorWrapper::BasicIterator::BasicIterator(const std::vector<std::vector<Document>> &c,
                                            size_t segment, size_t offset) : data_(c) {
    size_ = c.size();
    segment_ = segment;
    offset_ = offset;
}

[[nodiscard]] bool VectorWrapper::BasicIterator::operator==(const BasicIterator& rhs) const noexcept{
    return  (data_ == rhs.data_) &&
            (segment_ == rhs.segment_) &&
            (offset_ == rhs.offset_) &&
            (size_ == rhs.size_);
}

[[nodiscard]] bool VectorWrapper::BasicIterator::operator!=(const BasicIterator& rhs) const noexcept {
    return !(*this == rhs);
}

[[nodiscard]] Document VectorWrapper::BasicIterator::operator*() const noexcept {
    return data_[segment_][offset_];
}

VectorWrapper::BasicIterator& VectorWrapper::BasicIterator::operator++() noexcept {
    if (offset_ < (data_[segment_].size() - 1)) {
        ++offset_;
    } else {
        if (segment_ < (size_ - 1)) {
            ++segment_;
            offset_ = 0;
        }
        else {
            ++offset_;
        }
    }
    return *this;
}

VectorWrapper::BasicIterator VectorWrapper::BasicIterator::operator++(int) noexcept {
            auto old_value = *this; // Сохраняем прежнее значение объекта для последующего возврата
            ++(*this); // используем логику префиксной формы инкремента
    return old_value;
}

VectorWrapper::VectorWrapper(std::vector<std::vector<Document>> &&data) {
    data_ = std::move(data);
}
