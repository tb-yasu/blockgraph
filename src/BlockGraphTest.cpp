#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <sys/time.h>

#include "stdint.h"
#include "BlockGraph.hpp"

using namespace std;

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

int test1(int argc, char **argv){
  vector<uint8_t> str;
  str.push_back('N'); //0
  str.push_back('N'); //1
  str.push_back('B'); //2
  str.push_back('O'); //3
  str.push_back('B'); //4
  str.push_back('O'); //5
  str.push_back('T'); //6
  str.push_back('W'); //7 
  str.push_back('N'); //8
  str.push_back('N'); //9
  str.push_back('B'); //10
  str.push_back('O'); //11
  str.push_back('B'); //12
  str.push_back('I'); //13
  str.push_back('O'); //14
  str.push_back('O'); //15
  str.push_back('T'); //16
  str.push_back('B'); //17
  str.push_back('S'); //18
  str.push_back('H'); //19
  str.push_back('T'); //20
  str.push_back('F'); //21
  str.push_back('N'); //22
  str.push_back('E'); //23
  str.push_back('B'); //24
  str.push_back('O'); //25
  str.push_back('B'); //26
  str.push_back('O'); //27
  str.push_back('T'); //28
  str.push_back('W'); //29
  str.push_back('N'); //30
  str.push_back('E'); //31
  str.push_back('B'); //32
  str.push_back('O'); //33
  str.push_back('B'); //34
  str.push_back('O'); //35
  str.push_back('T'); //36
  str.push_back('W'); //37
  str.push_back('N'); //38
  str.push_back('E'); //39
  str.push_back('B'); //40
  str.push_back('O'); //41
  str.push_back('B'); //42
  str.push_back('I'); //43
  str.push_back('O'); //44
  str.push_back('O'); //45
  str.push_back('T'); //46
  str.push_back('B'); //47
  str.push_back('S'); //48
  str.push_back('H'); //49
  str.push_back('T'); //50
  str.push_back('F'); //51
  str.push_back('N'); //52
  str.push_back('S'); //53
  str.push_back('B'); //54
  str.push_back('O'); //55
  str.push_back('B'); //56
  str.push_back('O'); //57
  str.push_back('T'); //58
  str.push_back('W'); //59

  vector<uint8_t> tmp = str;
  BlockGraph bg;
  bg.buildBlockGraph(str, 2, 4, true);

  cout << "size (byte) : " << bg.getBytes() << " " << sizeof(uint8_t) * str.size() << endl;

  vector<uint8_t> res;
  bg.access(0, 59, res);
  for (size_t i = 0; i < res.size(); ++i) {
    if (res[i] != tmp[i]) 
      cerr << "error" << endl;
  }

  for (size_t i = 0; i < tmp.size(); ++i) {
    if (bg.get(i) != tmp[i]) 
      cout << i << " " << bg.get(i) << " " << tmp[i] << endl;
  }

  for (size_t i = 0; i < tmp.size(); ++i) 
    cout << "rank T : " << i << " " << bg.rank(i, 'T') << endl;

  cout << "select T : " << 3 << " " << bg.select(3, 'T') << endl;
}

int test2(int argc, char **argv) {
  ifstream ifs(argv[1]);
  vector<uint8_t> str;
  string line;
  while (getline(ifs, line)) {
    for (size_t i = 0; i < line.size(); ++i)
      str.push_back(line[i]);
  }

  uint64_t num = 100;
  uint64_t height = 3;
  cout << "access test" << endl;
  for (size_t arity = 5; arity <= 10; arity+=2) {
    BlockGraph bg;
    double stime = gettimeofday_sec();
    bg.buildBlockGraph(str, arity, height, false);
    cout << "arity : "<< arity << " height : " << height << " size (MB) : " << bg.getBytes()/1024.f/1024.f << endl;
    cout << "construction time (sec): "<< gettimeofday_sec() - stime << endl;
    stime = gettimeofday_sec();
    for (size_t i = 0; i < num; ++i) {
      bg.get(rand()%str.size());
    }
    cout << "access time : " << (gettimeofday_sec() - stime)/(double)num << endl;
  }
}

int main(int argc, char **argv) {
  //  test1(argc, argv);
  test2(argc, argv);

  
}

