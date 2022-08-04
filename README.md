# MySkiplist

## Introduction
Linux下基于skiplist实现的使用C++开发的轻量级键值数据库，功能有插入数据、删除数据、查询数据、数据展示、数据落盘以及文件加载数据。

在随机写读情况下，该项目每秒可处理请求数 (QPS): 12.97w

## Technical points

* 查询
    * 查询是插入和删除操作的基础，跳表的查询、插入和删除操作的平均时间复杂度是 `O(logn)`, 跳表的期望空间复杂度为 `O(n)`。
    * 在跳表中查询从最高层开始，水平地逐个比较直至当前节点的下一个节点大于等于目标节点，然后移动至下一层。重复这个过程直至到达第一层且无法继续进行操作。此时，若下一个节点是目标节点，则成功查找；反之，则元素不存在。
    ```C++
    for (int i = this->skiplist_level; i >= 0; i--) {
        while (cur->next[i] != nullptr && cur->next[i]->get_key() < key) {
            cur = cur->next[i];
        }
    }
    ```
* 插入
    * 插入操作是在查询的基础上进行的，同时需要保存每层下降的节点以便后续更改节点指向。
    ```C++
    for (int i = this->skiplist_level; i >= 0; i--) {
        while (cur->next[i] != nullptr && cur->next[i]->get_key() < key) {
            cur = cur->next[i];
        }
        pre[i] = cur;
    }
    ```
    * 理想状况下，我们希望最底层节点每两个之间就有一个索引，所以每次新插入元素的时候，尽量让该元素有 1/2 的几率建立一级索引、1/4 的几率建立二级索引、1/8 的几率建立三级索引，以此类推... 我们可以实现一个 `get_random_level()` 方法，该方法会随机生成 1~MAX_LEVEL 之间的数，且该方法有 1/2 的概率返回 1、1/4 的概率返回 2、1/8的概率返回 3，以此类推。
    ```C++
    int get_random_level() {
        int k = 1;
        while (rand() % 2) {
            k++;
        }
        k = min(k, max_level);
        return k;
    }
    ```
    * 需注意如果我们新插入节点的索引高度大于当前最高索引高度时，应该将头结点指向高出的部分。
* 删除
    * 删除操作同理也需要保存每层下降的节点以便后续更改节点指向。
    * 注意，改变前驱节点指向是应从底层开始向上进行，当到达某层节点并不指向需要删除的节点时就可以停止了(因为已经到达目标节点的最高索引高度了，更上层的节点肯定不会再指向它）。
    ```C++
    for (int i = 0; i < skiplist_level + 1; i++) {
            if (pre[i]->next[i] != cur) 
                break;
            pre[i]->next[i] = cur->next[i];
        }
    ```
    * 同时也需注意删除后可能需要降低当前表的高度，例如删除了索引高度最高的节点。
    ```C++
    while (skiplist_level > 0 && header->next[skiplist_level] == nullptr) {
            skiplist_level--;
        }
    ```
* 总结
    * 跳表是可以实现二分查找的有序链表；
    * 每个元素插入时随机生成它的 level；
    * 最底层包含所有的元素（普通的有序链表）；
    * 如果一个元素出现在 level(x)，那么它肯定出现在 x 以下的 level 中；
    * 跳表查询、插入、删除的时间复杂度为 `O(logn)`，与平衡二叉树接近；
* 为什么 Redis 选择使用跳表而不是红黑树来实现有序集合？
    * Redis 中的有序集合(zset) 支持的操作：插入一个元素；删除一个元素；查找一个元素；有序输出所有元素；按照范围区间查找元素（比如查找值在 [100, 356] 之间的数据）；
    * 其中，前四个操作红黑树也可以完成，且时间复杂度跟跳表是一样的。但是，按照区间来查找数据这个操作，红黑树的效率没有跳表高。按照区间查找数据时，跳表可以做到 `O(logn)` 的时间复杂度定位区间的起点，然后在原始链表中顺序往后遍历就可以了，非常高效。

## Index tree
```
.
├── LICENSE
├── main.cpp
├── makefile
├── README.md
├── skiplist.h
├── store
│   └── dumpfile
└── test
    └── stress_test.cpp
```

## Build and Run

* 修改 skiplist.h 中的默认文件位置
    ```C++
    #define STORE_FILE "store/dumpfile"
    ```

* 生成

    ```
    make 
    ```

* 启动

    ```
    ./run
    ```
## API
* search_element（查询数据）

* insert_element（插入数据)

* delete_element（删除数据）

* display_list（数据显示）

* get_size（获取数据库规模）

* dump_file（数据落盘）

* void load_file（加载数据）

## Test

test enviroment：
* Ubuntu 22.04 LTS on remote VMware
* RAM：4GB

|data numbers|time(sec)|
|:---:|:---:|
|100,000|0.771596|
|500,000|3.76952|
|1,000,000|8.60744|

每秒可处理写请求数（QPS）: 12.97w

## Thanks
@Carl: https://github.com/youngyangyang04/Skiplist-CPP