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
#include <string>
#include <map>
#include <cmath>
#include <stdint.h>

#include "RabinKarp.hpp"
#include "RSDicBuilder.hpp"
#include "RSDic.hpp"

struct Node {
  uint64_t offset_;
  uint64_t pos_;
  std::vector<uint32_t> rank;
  
  Node() {
    offset_ = pos_ = 0ULL;
  }

  void load(std::istream &is) {
    is.read((char*)(&offset_), sizeof(offset_));
    is.read((char*)(&pos_), sizeof(pos_));
    
    {
      size_t size;
      is.read((char*)(&size), sizeof(size));
      rank.resize(size);
      is.read((char*)(&rank[0]), sizeof(uint32_t) * size);
    }
  }
  void save(std::ostream &os) {
    os.write((const char*)(&offset_), sizeof(offset_));
    os.write((const char*)(&pos_), sizeof(pos_));
    {
      size_t size;
      os.write((const char*)(&size), sizeof(size));
      rank.resize(size);
      os.write((const char*)(&rank[0]), sizeof(uint32_t) * size);
    }
  }

  uint64_t getBytes() {
    size_t size = 0;
    size += sizeof(offset_);
    size += sizeof(pos_);
    size += sizeof(uint32_t) * rank.size();
    return size;
  }
};

template <typename Type>
struct Leaf {
  std::vector<Type> str;
  std::vector<uint32_t> rank;

  void load(std::istream &is) {
    {
      size_t size;
      is.read((char*)(&size), sizeof(size));
      str.resize(size);
      is.read((char*)(&str[0]), sizeof(Type) * size);
    }
    {
      size_t size;
      is.read((char*)(&size), sizeof(size));
      rank.resize(size);
      is.read((char*)(&rank[0]), sizeof(uint32_t) * size);
    }
  }
  void save(std::ostream &os) {
    {
      size_t size = str.size();
      os.write((const char*)(&size), sizeof(size));
      os.write((const char*)(&str[0]), sizeof(Type) * size);
    }
    {
      size_t size = rank.size();
      os.write((const char*)(&size), sizeof(size));
      os.write((const char*)(&rank[0]), sizeof(uint32_t) * size);
    }
  }
  uint64_t getBytes() {
    uint64_t size = 0;
    size += sizeof(Type) * size;
    size += sizeof(uint32_t) * rank.size();
    return size;
  }
};

class BlockGraph {
private:
  uint64_t compHeight(uint64_t blocklength, uint64_t arity);
  void     compMaxVal(const std::vector<uint8_t> &str);
  void     markBlock(std::vector<uint8_t> &str, std::vector<std::pair<uint64_t, uint64_t> > &blocks, std::vector<std::pair<uint8_t, uint64_t> > &checker);
  void     buildBlockGraphRec(std::vector<uint8_t> &str, std::vector<std::pair<uint64_t, uint64_t> > &blocks, uint64_t level, std::vector<std::vector<uint32_t> > &ranks, bool rankflag);
  void     buildRSDics();
  void     adjustString(std::vector<uint8_t> &str);
  uint64_t getRec(uint64_t blockid, uint64_t pos, uint64_t blocklength, uint64_t level);
  void     accessRec(uint64_t blockid, uint64_t from, uint64_t to, uint64_t blocklength, uint64_t level, std::vector<uint8_t> &res);
  uint64_t rankRec(uint64_t blockid, uint64_t pos, uint8_t val, uint64_t blocklength, uint64_t level ,bool flag);
public:
  void     buildBlockGraph(std::vector<uint8_t> &str, uint64_t arity, uint64_t height, bool rankflag);
  uint64_t get(uint64_t i);
  void     access(uint64_t from, uint64_t to, std::vector<uint8_t> &res);
  uint64_t rank(uint64_t pos, uint8_t val);
  uint64_t select(uint64_t i, uint8_t val);
  void     load(std::istream &is);
  void     save(std::ostream &os);
  uint64_t getBytes();
private:
  std::vector<std::vector<Node> > nodes;
  std::vector<std::vector<Leaf<uint8_t> > > leaves;
  std::vector<rsdic::RSDicBuilder> rsdicdbs_;
  std::vector<rsdic::RSDic> rsdics_;
  uint64_t blocklength_;
  uint64_t arity_;
  uint64_t height_;
  uint64_t txtlength_;
  
  uint8_t NONE;
  static uint64_t NPOS;
};
