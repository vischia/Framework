#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>
#include <map>
#include <vector>

#include <cp3_llbb/Framework/interface/Cut.h>
#include <cp3_llbb/Framework/interface/ProducersManager.h>
#include <cp3_llbb/Framework/interface/AnalyzersManager.h>

#include <cp3_llbb/TreeWrapper/interface/TreeWrapper.h>

class CategoryManager;

struct CategoryMetadata {
    // Empty
};

class Category {
    public:
        virtual bool event_in_category_pre_analyzers(const ProducersManager& producers) const = 0;
        virtual bool event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const = 0;

        virtual void register_cuts(CutManager& manager) {};

        virtual void evaluate_cuts_pre_analyzers(CutManager& manager, const ProducersManager& producers) const {};
        virtual void evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {};

        virtual std::shared_ptr<CategoryMetadata> get_metadata() final {
            return metadata;
        };

    protected:
        std::shared_ptr<CategoryMetadata> metadata;
};

struct CategoryData {
    std::string name;
    std::string description;
    std::unique_ptr<Category> callback;
    ROOT::TreeGroup tree;
    CutManager cut_manager;

    uint64_t events = 0;

    // Tree branches
    bool in_category_pre = false;
    bool in_category_post = false;
    bool& in_category;

    CategoryData(const std::string& name_, const std::string& description_, std::unique_ptr<Category> category, ROOT::TreeWrapper& tree_):
        name(name_),
        description(description_),
        callback(std::move(category)),
        tree(tree_.group(name_ + "_")),
        cut_manager(*this),
        in_category(tree["category"].write<bool>())
    {
        callback->register_cuts(cut_manager);
    }
};

class CategoryManager {
    friend class ExTreeMaker;

    public:
        template<class T>
        void new_category(const std::string& name, const std::string& description) {
            std::unique_ptr<Category> category(new T());
            m_categories.push_back(CategoryData(name, description, std::move(category), m_tree));
        }

        bool evaluate_pre_analyzers(const ProducersManager& producers);
        bool evaluate_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers);

        void reset();

        void print_summary();

    private:
        CategoryManager(ROOT::TreeWrapper& tree):
            m_tree(tree)
        {
            // Empty
        }

        std::vector<CategoryData> m_categories;
        ROOT::TreeWrapper& m_tree;

        uint64_t processed_events = 0;
        uint64_t selected_events = 0;
};

#endif
