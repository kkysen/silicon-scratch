//
// Created by Khyber on 1/17/2019.
//

#ifndef SiliconScratch_iterators_H
#define SiliconScratch_iterators_H

namespace iterators {
    
    template <typename Container>
    typename Container::iterator removeConst(typename Container::const_iterator iterator) {
        Container empty;
        return empty.erase(iterator, iterator);
    }
    
}

#endif // SiliconScratch_iterators_H
