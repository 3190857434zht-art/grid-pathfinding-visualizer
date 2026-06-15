#pragma once

#include "core/Grid.h"

#include <algorithm>
#include <string>
#include <vector>

namespace pathplanner {

enum class SearchEventType {
    NodeVisited,
    NodeExpanded,
    PathFound,
    SearchFinished
};

struct SearchEvent {
    SearchEventType type = SearchEventType::NodeVisited;
    Point point{};
    int step = 0;
    std::string message;
};

class SearchObserver {
public:
    virtual ~SearchObserver() = default;
    virtual void onSearchEvent(const SearchEvent& event) = 0;
};

class SearchSubject {
public:
    void addObserver(SearchObserver* observer);
    void removeObserver(SearchObserver* observer);

protected:
    void notify(const SearchEvent& event);

private:
    std::vector<SearchObserver*> observers_;
};

}  // namespace pathplanner
