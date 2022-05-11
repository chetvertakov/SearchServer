#pragma once

#include "document.h"
#include "search_server.h"

#include <random>
#include <string>
#include <vector>

void AddDocument(SearchServer &search_server, int document_id, const std::string &document,
                 DocumentStatus status = DocumentStatus::ACTUAL, const std::vector<int> &ratings = {});

void FindTopDocuments(const SearchServer &search_server, const std::string &raw_query);


std::string GenerateWord(std::mt19937& generator, int max_length);

std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length);

std::string GenerateQuery(std::mt19937& generator,
                          const std::vector<std::string>& dictionary, int max_word_count, double minus_prob);

std::vector<std::string> GenerateQueries(std::mt19937& generator,
                                         const std::vector<std::string>& dictionary,
                                         int query_count, int max_word_count);


void ProfileGetWordFrequencies();
void ProfileRemoveDocument();
