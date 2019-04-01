/*
  rexgen - a tool to create words based on regular expressions
  Copyright (C) 2012-2017  Jan Starke <jan.starke@outofbed.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110, USA
*/


#ifndef SRC_LIBREXGEN_ITERATOR_TERMINALREGEXITERATOR_H_
#define SRC_LIBREXGEN_ITERATOR_TERMINALREGEXITERATOR_H_

#include <librexgen/iterator/iterator.h>
#include <librexgen/debug.h>
#include <librexgen/string/unicode.h>
#include <librexgen/string/simplestring.h>
#include <librexgen/state/invaliditeratoridexception.h>
#include <string.h>
#include <string>
namespace rexgen {
  class TerminalRegexIterator : public Iterator {
  public:
    TerminalRegexIterator(const wchar_t *_terminal, size_t elements)
            : Iterator() {
      for (size_t idx = 0; idx < elements; ++idx) {
        append_widechar(terminal, _terminal[idx]);
      }
    }

    bool next() override {
      const bool res = (state == resetted);
      state = usable;
      return res;
    }

    void value(std::string& dst) const override {
      dst.append(terminal);
    }

    bool hasNext() const override { return state == resetted; }

    void updateReferences(IteratorState& /* iterState */) override {}

    void updateAttributes(IteratorState& /* iterState */) override {}

  private:
    std::string terminal;
  };
}
#endif  // SRC_LIBREXGEN_ITERATOR_TERMINALREGEXITERATOR_H_
