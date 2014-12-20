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

#include "BlockGraph.hpp"

using namespace::std;

uint64_t BlockGraph::NPOS = 0xffffffffffffffff;

void BlockGraph::compMaxVal(const vector<uint8_t> &str) {
  uint64_t maxVal = 0;
  for (size_t i = 0; i != str.size(); ++i) {
    if (maxVal < str[i])
      maxVal = str[i];
  }
  NONE = maxVal + 1;
}

void BlockGraph::adjustString(std::vector<uint8_t> &str) {
  if (str.size()%blocklength_ == 0)
    return;
  uint64_t div = str.size()/blocklength_;
  uint64_t len = blocklength_ * (div + 1);
  uint64_t size = str.size();
  str.resize(len);
  for (size_t i = size; i != len; ++i) 
    str[i] = NONE;
}

uint64_t BlockGraph::get(uint64_t i) {
  return getRec(i/blocklength_, i%blocklength_, blocklength_, 0);
}

uint64_t BlockGraph::getRec(uint64_t blockid, uint64_t pos, uint64_t blocklength, uint64_t level) {
  if (rsdics_[level].GetBit(blockid) == 0) {
    if (level == height_) { // leaf
      uint64_t num1 = rsdics_[level].Rank(blockid, 1);
      return leaves[level][blockid - num1].str[pos];
    }
    uint64_t num0 = rsdics_[level].Rank(blockid, 0);
    uint64_t childblocklength = blocklength/arity_;
    return getRec(num0*arity_ + pos/childblocklength, pos%childblocklength, childblocklength, level + 1);
  }
  uint64_t nodeid = rsdics_[level].Rank(blockid, 1);
  Node &node = nodes[level][nodeid];
  uint64_t o = node.offset_;
  uint64_t p = node.pos_ + pos;
  if (p >= blocklength) {
    ++o; p %= blocklength;
  }
   
  return getRec(o, p, blocklength, level);
}

void BlockGraph::access(uint64_t from, uint64_t to, vector<uint8_t> &res) {
  uint64_t epos = from;
  for (size_t spos = from; spos <= to; ++spos) {
    uint64_t id = spos/blocklength_;
    while (++epos <= to && epos%blocklength_ != 0) {};
    accessRec(id, spos%blocklength_, (epos - 1)%blocklength_, blocklength_, 0, res);
    spos = epos - 1;
  }
}

void BlockGraph::accessRec(uint64_t blockid, uint64_t from, uint64_t to, uint64_t blocklength, uint64_t level, vector<uint8_t> &res) {
  if (rsdics_[level].GetBit(blockid) == 0) {
    if (level == height_) {
      uint64_t num1 = rsdics_[level].Rank(blockid, 1);
      for (uint64_t pos = from; pos <= to; ++pos) 
	res.push_back(leaves[level][blockid - num1].str[pos]);
      return;
    }
    uint64_t arity = rsdics_[level].Rank(blockid, 0) * arity_;
    uint64_t childblocklength = blocklength/arity_;
    for (size_t pos = from; pos <= to; ++pos) {
      uint64_t id   = pos/childblocklength;
      uint64_t spos = pos%childblocklength;
      while (++pos <= to && pos%childblocklength != 0) {}
      --pos;
      uint64_t epos = pos%childblocklength;
      accessRec(arity + id, spos, epos, childblocklength, level + 1, res);
    }
    return;
  }

  uint64_t nodeid = rsdics_[level].Rank(blockid, 1);
  Node &node = nodes[level][nodeid];
  uint64_t offset = node.offset_;
  uint64_t length = to - from + 1; 
  uint64_t frompos = node.pos_ + from;
  for (size_t p = 0; p < length; ++p) {
    uint64_t topos = frompos;
    while (++p < length && (++topos)%blocklength != 0) {}
    if (p == length)
      topos++;
    accessRec(offset, frompos, topos - 1, blocklength, level, res);
    --p;
    frompos = 0;
    offset++;
  }
}

uint64_t BlockGraph::rank(uint64_t pos, uint8_t val) {
  return rankRec(pos/blocklength_, pos%blocklength_, val, blocklength_, 0, false);
}

uint64_t BlockGraph::rankRec(uint64_t blockid, uint64_t pos, uint8_t val, uint64_t blocklength, uint64_t level ,bool flag) {
  if (rsdics_[level].GetBit(blockid) == 0) {
    if (level == height_) { // leaf
      if (rsdics_[level].num() == blockid) {
	blockid--;
	pos = blocklength;
      }
      uint64_t num1  = rsdics_[level].Rank(blockid, 1);
      uint64_t count = leaves[level][blockid - num1].rank[val];
      if (flag)
	count = 0;
      for (size_t i = 0; i != pos; ++i) {
	if (leaves[level][blockid-num1].str[i] == val)
	  count++;
      }
      return count;
    }
    uint64_t num0 = rsdics_[level].Rank(blockid, 0);
    uint64_t childblocklength = blocklength/arity_;

    uint64_t rank = 0;
    if (flag) {
      for (size_t i = 0, len = pos/childblocklength; i != len; ++i)
	rank += rankRec(num0*arity_ + i, childblocklength, val, childblocklength, level + 1, flag);
    }
    return rankRec(num0*arity_ + pos/childblocklength, pos%childblocklength, val, childblocklength, level + 1, flag) + rank;

  }
  uint64_t nodeid = rsdics_[level].Rank(blockid, 1);
  Node &node = nodes[level][nodeid];
  uint64_t o = node.offset_;
  uint64_t p = node.pos_ + pos;
  uint64_t rank = 0;
  if (!flag)
    rank = node.rank[val];
  return rankRec(o, p, val, blocklength, level, true) + rank;
}

uint64_t BlockGraph::select(uint64_t i, uint8_t val) {
  uint64_t left  = 0;
  uint64_t right = txtlength_;
  while (left < right) {
    uint64_t pivot = (left + right)/2;
    uint64_t rank  = (this)->rank(pivot, val);
    if     (i < rank) { right = pivot; }
    else if(i > rank) { left  = pivot + 1; }
    else {
      while ((pivot > 0) && (get(pivot) != val)) 
	pivot++;
      return pivot + 1;
    }
  }
  return NPOS;
}

void BlockGraph::buildRSDics() {
  rsdics_.resize(rsdicdbs_.size());
  for (size_t i = 0; i < rsdics_.size(); ++i)
    rsdicdbs_[i].Build(rsdics_[i]);
}

uint64_t BlockGraph::compHeight(uint64_t blocklength, uint64_t arity) {
  uint64_t height = 0;
  while (blocklength>arity) {
    blocklength/=arity;
    height++;
  }
  return height;
}

void BlockGraph::markBlock(vector<uint8_t> &str, vector<pair<uint64_t, uint64_t> > &blocks, vector<pair<uint8_t, uint64_t> > &checker) {
  uint64_t pLen = 0;
  for (size_t i = 0; i < blocks.size() - 1; ++i) {
    pair<uint64_t, uint64_t> &block1 = blocks[i];
    pair<uint64_t, uint64_t> &block2 = blocks[i + 1];
    if (block1.second + 1 == block2.first) {
      pLen = block2.second - block1.first + 1;
      break;
    }
  }
  vector<uint64_t> patterns;
  RabinKarp<uint8_t> rk;
  for (size_t i = 0; i < blocks.size() - 1; ++i) {
    pair<uint64_t, uint64_t> &block1 = blocks[i];
    pair<uint64_t, uint64_t> &block2 = blocks[i + 1];
    if (block1.second + 1 == block2.first)
      rk.addElements(str, block1.first, pLen, i);
  }
  vector<pair<uint64_t, uint64_t> > res;
  rk.patternMatch(str, pLen, res);
  checker.resize(blocks.size());
  bool flag = true;
  for (size_t i = 0; i != res.size(); ++i) {
    uint64_t id = res[i].first;
    checker[id].first++; checker[id + 1].first++;
    checker[id].second = res[i].second;
    if (flag) {
      checker[id].first++;
      flag = false;
    }
    if (id + 1 < blocks.size() && blocks[id].second + 1 != blocks[id+1].first)
      flag = true;
    if (id < blocks.size() && blocks[id + 1].second + 1 == str.size())
      checker[id + 1].first++;
  }
}

void BlockGraph::buildBlockGraphRec(vector<uint8_t> &str, vector<pair<uint64_t, uint64_t> > &blocks, uint64_t level, vector<vector<uint32_t> > &ranks, bool rankflag) {
  vector<pair<uint8_t, uint64_t> > checker;
  markBlock(str, blocks, checker);
  nodes.resize(nodes.size() + 1);
  leaves.resize(leaves.size() + 1);
  rsdicdbs_.resize(rsdicdbs_.size() + 1);
  vector<uint64_t> blockcheker;
  for (size_t i = 0; i != checker.size(); ++i) {
    if (checker[i].first == 2) {
      uint64_t respos = checker[i].second;
      Node node;
      size_t nodeid;
      for (nodeid = 0; nodeid != blocks.size(); ++nodeid) {
	if (blocks[nodeid].first <= respos && respos <= blocks[nodeid].second) {
	  node.offset_ = respos/(blocks[nodeid].second - blocks[nodeid].first + 1);
	  node.pos_    = respos%(blocks[nodeid].second - blocks[nodeid].first + 1);
	  break;
	}
      }
      if (rankflag) 
	node.rank = ranks[i];
      nodes[nodes.size() - 1].push_back(node);
      rsdicdbs_[rsdicdbs_.size() - 1].PushBack(1);
    }
    else {
      rsdicdbs_[rsdicdbs_.size() - 1].PushBack(0);
      blockcheker.push_back(i);
      if (level == height_) {
	uint64_t spos = blocks[i].first;
	uint64_t epos = blocks[i].second;
	Leaf<uint8_t> leaf;
	for (size_t j = spos; j <= epos; ++j) 
	  leaf.str.push_back(str[j]);
	if (rankflag) 
	  leaf.rank = ranks[i];
	leaves[leaves.size() - 1].push_back(leaf);
      }
    }
  }
  if (level == height_)
    return;

  vector<pair<uint64_t, uint64_t> > newblocks;
  vector<vector<uint32_t> > newranks;
  for (size_t i = 0; i < blockcheker.size(); ++i) {
    uint64_t id   = blockcheker[i];
    uint64_t spos = blocks[id].first;
    uint64_t epos = blocks[id].second;
    uint64_t blocklength = epos - spos + 1;
    uint64_t len = blocklength/arity_;
    uint64_t start = spos;
    vector<uint32_t> counter;
    if (rankflag)
      counter = ranks[id];
    for (size_t j = 0; j != arity_; ++j) {
      newblocks.push_back(make_pair(start, start + len - 1));
      if (rankflag) {
	newranks.push_back(counter);
	for (size_t k = start; k <= start + len - 1; ++k) {
	  if (str[k] >= counter.size())
	    counter.resize(str[k] + 1);
	  counter[str[k]]++;
	}
      }
      start += len;
    }
  }

  for (size_t i = 0; i != checker.size(); ++i) {
    if (checker[i].first == 2) {
      uint64_t spos = blocks[i].first;
      uint64_t epos = blocks[i].second;
      for (size_t j = spos; j <= epos; ++j)
	str[j] = NONE;
    }
  }

  vector<pair<uint8_t, uint64_t> >().swap(checker);
  vector<pair<uint64_t, uint64_t> >().swap(blocks);
  buildBlockGraphRec(str, newblocks, level + 1, newranks, rankflag);
}

void BlockGraph::buildBlockGraph(std::vector<uint8_t> &str, uint64_t arity, uint64_t height,   bool rankflag) {
  arity_  = arity;
  height_ = height;
  blocklength_ = pow(arity_, height_ + 1);
  compMaxVal(str);
  adjustString(str);
  txtlength_ = str.size();
  vector<pair<uint64_t, uint64_t> > blocks;
  vector<vector<uint32_t> > ranks;
  uint64_t num = ceil(str.size()/blocklength_);

  vector<uint32_t> counter(NONE);
  uint64_t start = 0;
  for (size_t i = 0; i != num; ++i) {
    blocks.push_back(make_pair(start, start + blocklength_ - 1));
    if (rankflag) {
      ranks.push_back(counter);
      for (size_t j = start; j <= start + blocklength_ - 1; ++j) {
	if (counter.size() <= str[j]) 
	  counter.resize(str[j] + 1);
	counter[str[j]]++;
      }
    }
    start += blocklength_;
  }
  if (blocks[blocks.size()-1].second < str.size() - 1) {
    blocks.push_back(make_pair(blocks[blocks.size()-1].second+1, str.size()-1));
    if (rankflag)
      ranks.push_back(counter);
  }

  buildBlockGraphRec(str, blocks, 0, ranks, rankflag);
  buildRSDics();
}

void BlockGraph::load(istream &is) {
  {
    size_t size;
    is.read((char*)(&size), sizeof(size));
    nodes.resize(size);
    for (size_t i = 0; i != size; ++i) {
      size_t size2;
      is.read((char*)(&size2), sizeof(size2));
      nodes[i].resize(size2);
      for (size_t j = 0; j != size2; ++j)
	nodes[i][j].load(is);
    }
  }
  {
    size_t size;
    is.read((char*)(&size), sizeof(size));
    leaves.resize(size);
    for (size_t i = 0; i != size; ++i) {
      size_t size2;
      is.read((char*)(&size2), sizeof(size2));
      leaves[i].resize(size2);
      for (size_t j = 0; j != size2; ++j)
	leaves[i][j].load(is);
    }
  }
  {
    size_t size;
    is.read((char*)(&size), sizeof(size));
    rsdics_.resize(size);
    for (size_t i = 0; i != size; ++i)
      rsdics_[i].Load(is);
  }
  is.read((char*)(&blocklength_), sizeof(blocklength_));
  is.read((char*)(&arity_), sizeof(arity_));
  is.read((char*)(&height_), sizeof(height_));
  is.read((char*)(&txtlength_), sizeof(txtlength_));
  is.read((char*)(&NONE), sizeof(NONE));
}

void BlockGraph::save(ostream &os) {
  {
    size_t size = nodes.size();
    os.write((const char*)(&size), sizeof(size));
    for (size_t i = 0; i != size; ++i) {
      size_t size2 = nodes[i].size();
      os.write((const char*)(&size2), sizeof(size2));
      for (size_t j = 0; j != size2; ++j) 
	nodes[i][j].save(os);
    }
  }
  {
    size_t size = leaves.size();
    os.write((const char*)(&size), sizeof(size));
    for (size_t i = 0; i != size; ++i) {
      size_t size2 = leaves[i].size();
      os.write((const char*)(&size2), sizeof(size2));
      for (size_t j = 0; j != size2; ++j) 
	leaves[i][j].save(os);
    }
  }
  {
    size_t size = rsdics_.size();
    os.write((const char*)(&size), sizeof(size));
    for (size_t i = 0; i != size; ++i) 
      rsdics_[i].Save(os);
  }
  os.write((const char*)(&blocklength_), sizeof(blocklength_));
  os.write((const char*)(&arity_), sizeof(arity_));
  os.write((const char*)(&height_), sizeof(height_));
  os.write((const char*)(&txtlength_), sizeof(txtlength_));
  os.write((const char*)(&NONE), sizeof(NONE));
}

uint64_t BlockGraph::getBytes() {
  uint64_t size = 0;
  for (size_t i = 0; i != nodes.size(); ++i) {
    for (size_t j = 0; j != nodes[i].size(); ++j)
      size += nodes[i][j].getBytes();
  }
  for (size_t i = 0; i != leaves.size(); ++i) {
    for (size_t j = 0; j != leaves[i].size(); ++j)
      size += leaves[i][j].getBytes();
  }
  for (size_t i = 0; i != rsdics_.size(); ++i)
    size += rsdics_[i].GetUsageBytes();
  size += 5 * sizeof(uint64_t);
  size += sizeof(uint8_t);
  return size;
}
