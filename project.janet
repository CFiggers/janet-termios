(declare-project
  :name "janet-termios"
  :description "TODO: Write a cool description"
  :license "MIT"
  :dependencies [])

(declare-native 
  :name "janet-termios"
  :cflags ["-Wall" "-Wextra"]
  :lflags ["-pthread"]
  :source ["janet_termios.c"])