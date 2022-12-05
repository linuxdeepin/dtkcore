\mainpage dtkcore
@brief dtk

# dtkcore

## 简介

dtkcore 是一个基于Qt的C++库，它提供了一些常用的工具类，以及一些基础的模块，如日志、插件、网络、线程、数据库、文件、图形、音频、视频、系统信息等

## 使用

现在的dtkcore>=5.6版本使用cmake来管理各个模块，所以使用dtkcore时，需要先安装cmake，然后需要在你的cmake项目中引入dtkcore的cmake模块，如下：

```cmake
find_package(DtkCore REQUIRED)

target_include_directories(
    ${DtkCore_INCLUDE_DIRS}
)

target_link_libraries(
    ${DtkCore_LIBRARIES}
)
```

以上示例仅为最小示例，并不能单独作为camke项目使用，需要你自己添加其他的cmake模块，如Qt的cmake模块，以及你自己的cmake模块.
@note 注意：dtkcore的cmake模块会自动引入Qt5的cmake模块，所以不需要再次引入Qt5的cmake模块.

## 文档

阅读文档建议从模块页面开始，模块页面提供了dtkcore的各个模块的简介，以及各个模块的使用示例。
@subpage DLog

dtkcore的文档使用doxygen管理,由deepin_doc_doc_go_sig提供维护支持, 如果你也想加入sig,请访问[deepin_doc_doc_go_sig](https://matrix.to/#/#deepin_doc_doc_go:matrix.org)

## 许可

dtkcore使用LGPLv3许可证，你可在此许可证下自由使用dtkcore <br>
dtkcore的文档使用[CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/)许可证，你可在此许可证下自由使用dtkcore的文档,但是转发或者引用时必须注明出处。
