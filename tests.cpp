#include <gtest/gtest.h>
#include <vector>
#include <map>
#include "hashmap.cpp"

TEST (hashmap, put){
  hashmap map;
  ASSERT_EQ(map.size(), 0);
  ASSERT_EQ(map.containsKey(2), false);
  //map.get(2);
  map.put(2, 30);
  ASSERT_EQ(map.size(), 1);
  ASSERT_EQ(map.containsKey(2), true);
  ASSERT_EQ(map.get(2), 30);
  map.put(3, 30);
  map.put(4, 30);
  ASSERT_EQ(map.containsKey(3), true);
  ASSERT_EQ(map.containsKey(4), true);
  ASSERT_EQ(map.get(3), 30);
  ASSERT_EQ(map.get(4), 30);
}