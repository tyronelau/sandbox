cc_library(name = "tinyxml",
           srcs = ["tiny*.cc"]
           )
cc_binary(name = "check_layouts",
          srcs = ["layout_checker.cc",
                  "ignore_list.cc",
                  "dag_sort.cc",
                  "util_main.cc"
                 ],
          deps = ["//base/common/BUILD:base",
                  "//base/file/BUILD:file",
                  "//base/strings/BUILD:strings",
                  ":tinyxml"]
          )

