#include "patterns/Observer.h"

namespace pathplanner {

void SearchSubject::addObserver(SearchObserver* observer) {
    if (observer != nullptr) {
        observers_.push_back(observer);
    }
}

void SearchSubject::removeObserver(SearchObserver* observer) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), observer),
        observers_.end());
}

void SearchSubject::notify(const SearchEvent& event) {
    for (SearchObserver* observer : observers_) {
        if (observer != nullptr) {
            observer->onSearchEvent(event);
        }
    }
}

}  // namespace pathplanner
