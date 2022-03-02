//
// util.h
//
// Michael Kulikowski
// UIC Cs251
//
// Text file compresser(Huffman)
//

#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <queue>       // std::priority_queue
#include <vector>      // std::vector
#include <functional>  // std::greater
#include <string>
#include "bitstream.h"
#include "util.h"
#include "hashmap.h"
#include "mymap.h"
#include <sstream>

struct HuffmanNode {
  int character;
  int count;
  HuffmanNode *zero;
  HuffmanNode *one;
};

class prioritize {
 public:
  bool operator()(const HuffmanNode *p1, const HuffmanNode *p2) const {
    return p1->count > p2->count;
  }
};
//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode *&node) {
  if (node == nullptr) {
    return;
  }
  freeTree(node->zero);
  freeTree(node->one);
  delete node;
}
//
// helper function for buildFrequencyMap
// adds to the count if find a matching key
//
void _frequencyBuilder(char c, hashmap &map) {
  if (map.containsKey(c)) {
    int freq = map.get(c);
    map.put(c, freq + 1);
  } else {
    map.put(c, 1);
  }
}
//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmap &map) {
  if (isFile) {
    ifstream inFS(filename);
    char c;
    while (inFS.get(c)) {
      _frequencyBuilder(c, map);
    }
  } else {
    for (char c : filename) {
      _frequencyBuilder(c, map);
    }
  }
  map.put(256, 1);
}
//
// Helper function for buildEncodingTree
// Parses through the map looking for details that it will assign nodes in the
// priority_queue
//
priority_queue<HuffmanNode *, vector<HuffmanNode *>, prioritize> parseAndQueue(
    hashmap &map) {
  priority_queue<HuffmanNode *, vector<HuffmanNode *>, prioritize> pq;
  stringstream ss;
  ss << map << endl;
  string key;
  string value;
  for (int i = 1; i < map.size(); i++) {
    getline(ss, key, ':');
    key.erase(0, 1);
    getline(ss, value, ',');
    HuffmanNode *node = new HuffmanNode;
    node->character = stoi(key);
    node->count = stoi(value);
    node->zero = nullptr;
    node->one = nullptr;
    pq.push(node);
  }
  getline(ss, key, ':');
  key.erase(0, 1);
  getline(ss, value, '}');
  HuffmanNode *node = new HuffmanNode;
  node->character = stoi(key);
  node->count = stoi(value);
  node->zero = nullptr;
  node->one = nullptr;
  pq.push(node);
  return pq;
}
//
// Prints priority_queue
// used for debugging
//
void printQueue(
    priority_queue<HuffmanNode *, vector<HuffmanNode *>, prioritize> q) {
  int N = q.size();
  HuffmanNode *cur;
  for (int i = 0; i < N; i++) {
    cur = q.top();
    cout << "Element: " << i << " Char: " << cur->character
         << " Count: " << cur->count << endl;
    q.pop();
  }
}
//
// Prints tree using recursion
// used for debugging
//
void printTree(HuffmanNode *node) {
  if (node == nullptr) {
    return;
  }
  printTree(node->zero);
  cout << " Char: " << node->character << " Count: " << node->count << endl;
  printTree(node->one);
}
//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode *buildEncodingTree(hashmap &map) {
  priority_queue<HuffmanNode *, vector<HuffmanNode *>, prioritize> pq;
  pq = parseAndQueue(map);
  HuffmanNode *node1;
  HuffmanNode *node2;
  while (pq.size() != 1) {
    HuffmanNode *node = new HuffmanNode;
    node->character = 257;
    node1 = pq.top();
    node->zero = node1;
    pq.pop();
    node2 = pq.top();
    node->one = node2;
    pq.pop();
    node->count = (node->zero->count) + (node->one->count);
    pq.push(node);
  }
  return pq.top();
}
//
// helper function for buildFrequencyMap
// finds appropriate character from binary string
//
void traverseTree(HuffmanNode *node, string ss,
                  mymap<int, string> &encodingMap) {
  if (node->character != 257) {
    encodingMap.put((int)node->character, ss);
    return;
  }
  string gg = ss + "0";
  traverseTree(node->zero, gg, encodingMap);
  string zz = ss + "1";
  traverseTree(node->one, zz, encodingMap);
}
//
// *This function builds the encoding map from an encoding tree.
//
mymap<int, string> buildEncodingMap(HuffmanNode *tree) {
  mymap<int, string> encodingMap;
  string ss = "";
  traverseTree(tree, ss, encodingMap);
  return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream &input, mymap<int, string> &encodingMap,
              ofbitstream &output, int &size, bool makeFile) {
  char c;
  string s = "";
  while (input.get(c)) {
    s += encodingMap.get((int)c);
  }
  s += encodingMap.get(256);
  if (makeFile == true) {
    int bValue;
    for (char c : s) {
      if (c == 48) {
        bValue = 0;
      } else if (c == 49) {
        bValue = 1;
      }
      output.writeBit(bValue);
      size++;
    }
  }
  return s;
}
//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode *encodingTree, ofstream &output) {
  string s;
  int bit;
  HuffmanNode *root = encodingTree;
  while (!input.eof()) {
    bit = input.readBit();
    if (bit == 0) {
      if (encodingTree->zero == nullptr) {
        if (encodingTree->character == 256) {
          break;
        }
        s += encodingTree->character;
        encodingTree = root;
      }
      encodingTree = encodingTree->zero;
    } else if (bit == 1) {
      if (encodingTree->one == nullptr) {
        if (encodingTree->character == 256) {
          break;
        }
        s += encodingTree->character;
        encodingTree = root;
      }
      encodingTree = encodingTree->one;
    }
  }
  for (char c : s) {
    output.put(c);
  }
  return s;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
  ofbitstream output(filename + ".huf");
  hashmap map;
  bool isFile = true;
  buildFrequencyMap(filename, isFile, map);

  HuffmanNode *tree = new HuffmanNode;
  tree = buildEncodingTree(map);

  mymap<int, string> encodingMap;
  encodingMap = buildEncodingMap(tree);

  ifstream input(filename);
  output << map;
  int size = 0;
  bool makeFile = true;
  string s;
  s = encode(input, encodingMap, output, size, makeFile);
  freeTree(tree);
  output.close();

  return s;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
  size_t pos = filename.find(".huf");
  if ((int)pos >= 0) {
    filename = filename.substr(0, pos);
  }
  pos = filename.find(".");
  string ext = filename.substr(pos, filename.length() - pos);
  filename = filename.substr(0, pos);
  ifbitstream input(filename + ext + ".huf");
  ofstream output(filename + "_unc" + ext);

  hashmap map;
  input >> map;

  HuffmanNode *tree = new HuffmanNode;
  tree = buildEncodingTree(map);

  string s;
  s = decode(input, tree, output);
  freeTree(tree);
  output.close();
  return s;
}
