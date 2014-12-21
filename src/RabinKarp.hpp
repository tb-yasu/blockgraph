/* 
 *  Copyright (c) 2014 Yasuo Tabei
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <fstream>
#include <map>
#include <cmath>
#include <stdint.h>
#include <unordered_map>
#include "RabinKarp.hpp"
#include "BlockGraphUtil.hpp"

template <typename Type>
class RabinKarp {
private:
  uint64_t compHash(std::vector<Type> &str, uint64_t from, uint64_t to, uint64_t prevHash) {
    uint64_t val;
    uint64_t len = std::min(to - from, 5ULL);

    if (from > 0) {
      val = prevHash;
      val -= str[from - 1] * pow(prime, len);
      val *= prime;
      val += str[to];
    }
    else {
      val = 0;
      for (size_t i = from; i <= to; ++i)  {
	if (len == 0)
	  break;
	val += str[i] * pow(prime, len--);
      }
    }
    return val;  
  }
public:
  RabinKarp() {
    prime = PRIME;
  }

  void addElements(std::vector<Type> &str, uint64_t pos, uint64_t pLen ,uint64_t id) {
    uint64_t hashVal = compHash(str, pos, pos + pLen - 1, 0);
    hashtables[hashVal].push_back(std::make_pair(pos, id));
  }

  void patternMatch(std::vector<Type> &str, uint64_t pLen, std::vector<std::pair<uint64_t, uint64_t> > &res) {
    if (str.size() < pLen)
      return;

    uint64_t hashval = compHash(str, 0, pLen - 1, 0);
    for (size_t i = 0; i <= str.size() - pLen; ++i) {
      //      if (i % 10000 == 0) {
	//	std::cout << i << " " << str.size() << std::endl;
      //      }
      std::unordered_map<uint64_t, std::vector<std::pair<uint64_t, uint64_t> > >::iterator it = hashtables.find(hashval);
      if (it != hashtables.end()) {
	std::vector<std::pair<uint64_t, uint64_t> > &ids = it->second;
        for (size_t j = 0; j < ids.size(); ++j) {
          uint64_t pos = ids[j].first;
          if (pos <= i)
            continue;
          uint64_t id  = ids[j].second;
          bool flag = true;
          for (size_t k = 0; k != pLen; ++k) {
            if (str[pos + k] != str[i + k]) {
              flag = false;
              break;
            }
          }
          if (flag)
            res.push_back(std::make_pair(id, i));
        }
      }
      if (i < str.size() - pLen)
        hashval = compHash(str, i + 1, i + pLen, hashval);
    }
  }
private:
  uint64_t prime;
  std::unordered_map<uint64_t, std::vector<std::pair<uint64_t, uint64_t> > > hashtables;
};
