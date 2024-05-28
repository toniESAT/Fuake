
#pragma once

#include <fstream>
#include <string>

using std::string;

namespace fuake {

struct TextFile {
   std::fstream fs;
   string buf;
   string path;

   bool ignore_empty = true;
   bool remove_newline;
   bool trim_trailing_whitespace;
   char comment_token = 0;

   void Close() { fs.close(); }

   void Open(const string filename, const string mode = "r") {

      int flags = 0;
      for (auto ch : mode) {
         switch (ch) {
         case 'r': flags |= std::fstream::in; break;
         case 'w': flags |= std::fstream::out; break;
         case 'b': flags |= std::fstream::binary; break;
         default: break;
         }
      }

      fs.open(filename, flags);

      buf.reserve(1024);

      int path_len = filename.find_last_of('/\\');

      path = filename.substr();
      path[path_len] = '\0';
   }

   void Reset() { fs.seekg(0); }

   bool ReadLine() {

      // Read until finding a line that is not a comment or empty
      do {
         getline(fs, buf);
      } while (
          ((ignore_empty && buf[0] == '\n') || (comment_token != 0 && buf[0] == comment_token)));

      if (fs.bad()) return false; // Return false if I/O error or EOF

      // Remove trailing newline character
      if (remove_newline && buf.back() == '\n') buf.back() = '\0';

      return true;
   }
};

} // namespace fuake