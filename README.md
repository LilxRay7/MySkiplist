# MySkiplist

## Introduction
Linux下基于skiplist实现的使用C++开发的轻量级键值数据库，功能有插入数据、删除数据、查询数据、数据展示、数据落盘以及文件加载数据。

## Index tree
```
.
├── LICENSE
├── main.cpp
├── makefile
├── README.md
├── skiplist.h
└── store
    └── dumpfile
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

## Thanks
@Carl: https://github.com/youngyangyang04/Skiplist-CPP