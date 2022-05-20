#include <clocale>
#include <fstream>
#include <iostream>
#include <codecvt>
#include <sstream>

#include "json.hpp"
#include "../Parser/Parser.h"
#include "Model.h"

std::vector<const CompoundModel *> loadModel(const std::string &filepath, Parser::Dictionary &dictionary) {
    std::ifstream input(filepath);
    nlohmann::json json;
    input >> json;

    std::unordered_map<std::string, VocabularyModel::NormalizedNGram> vocabulary;
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    const auto voc_json = json["vocabulary"];
    for (auto it = voc_json.begin(); it != voc_json.end(); it++) {
        const auto values = it.value().get<std::vector<std::string>>();
        for (const auto &nGram: values) {
            std::wstringstream ss(converter.from_bytes(nGram));
            const auto tokens = Parser::readTokens(ss, true);

            std::vector<const Parser::WordSet> normalizedValues;
            for (const auto &token: tokens) {
                const auto normalized = Parser::normalizeToken(token, dictionary);
                normalizedValues.push_back(normalized);
            }

            auto nGrams = vocabulary.find(it.key());
            if (nGrams == vocabulary.end()) {
                vocabulary.emplace(it.key(), VocabularyModel::NormalizedNGram());
                nGrams = vocabulary.find(it.key());
            }

            nGrams->second.insert(normalizedValues);
        }
    }

    std::vector<const CompoundModel *> models;
    std::unordered_map<std::string, const CompoundModel *> name2Model;

    const auto mod_json = json["models"];

    for (const auto &modelJSON: mod_json) {
        std::vector<const BaseModel *> baseModels;
        for (const auto &subModel: modelJSON["value"]) {
            if (subModel.contains("voc")) {
                const auto lemmas = vocabulary.at(subModel["voc"]);
                baseModels.push_back(new VocabularyModel(lemmas));
            } else if (subModel.contains("pos")) {
                baseModels.push_back(new PartOfSpeechModel(subModel["pos"]));
            } else if (subModel.contains("mod")) {
                const std::string modelName = subModel["mod"];
                if (name2Model.find(modelName) == name2Model.end()) {
                    throw std::runtime_error("Model \"" + modelName + "\" has not been loaded yet.");
                }
                baseModels.push_back(name2Model[modelName]);
            } else {
                throw std::runtime_error("Invalid base model");
            }
        }

        const auto model = new CompoundModel(modelJSON["name"], baseModels);
        models.push_back(model);
        name2Model[model->name] = model;
    }

    return models;
}

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");
    std::wcout.imbue(std::locale("ru_RU.UTF-8"));

    auto dictionary = Parser::readDictionary(DICTIONARY_PATH, 6000000);
    const auto iterator = std::filesystem::directory_iterator(NEWS_PATH);
    const auto models = loadModel(MODEL_PATH, dictionary);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::map<std::string, std::set<std::vector<std::wstring>>> fragments;

    for (const auto &entry: iterator) {
        std::wifstream wif(entry.path());
        const auto tokens = Parser::readTokens(wif, true);

        std::vector<const Parser::WordSet> normalizedContext;
        normalizedContext.reserve(tokens.size());

        for (const auto &token: tokens) {
            normalizedContext.push_back(Parser::normalizeToken(token, dictionary));
        }

        for (int j = 0, i = 0; j < tokens.size(); j++) {
            if (tokens[j] == L"." || tokens[j] == L"?" || tokens[j] == L"!") {
                for (const auto &model: models) {
                    std::set<int> matchedIndices;
                    const auto match = model->findMatch(
                            normalizedContext.begin() + i,
                            normalizedContext.begin() + j,
                            matchedIndices);
                    if (match != normalizedContext.begin() + j) {
                        std::vector<std::wstring> fragment;
                        for (int tokenPos = i; tokenPos != j; tokenPos++) {
                            auto token = *(tokens.begin() + tokenPos);
                            if (matchedIndices.find(tokenPos - i) != matchedIndices.end()) {
                                std::transform(token.begin(), token.end(),
                                               token.begin(), std::towupper);
                            }
                            fragment.push_back(token);
                        }
                        fragments[model->name].insert(fragment);
                    }
                }

                i = ++j;
            }
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "Fragment search duration = " << duration.count() << "[ms]" << std::endl << std::endl;

    for (const auto &pair: fragments) {
        std::cout << "Model «" << pair.first << "» matched " << pair.second.size() << " fragments:" << std::endl;
        for (const auto &fragment: pair.second) {
            std::cout << '\t';
            for (const auto &token: fragment) {
                std::wcout << token << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}