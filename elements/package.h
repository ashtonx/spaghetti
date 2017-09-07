// MIT License
//
// Copyright (c) 2017 Artur Wyszyński, aljen at hitomi dot pl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef ELEMENTS_PACKAGE_H
#define ELEMENTS_PACKAGE_H

#include <sparsepp/spp.h>
#include <concurrentqueue.h>

#include "core/id_manager.h"
#include "core/strings.h"
#include "elements/element.h"
#include "elements/types.h"

namespace elements {

class Package final : public Element {
 public:
  size_t type() const noexcept override { return Types::ePackage; }

  Element* add(char const *const a_name) { return add(string::hash(a_name)); }
  Element* add(string::hash_t a_hash);
  void remove(size_t a_id);
  Element* get(size_t a_id) const;

  bool connect(size_t a_sourceId, uint8_t a_outputId, size_t a_targetId, uint8_t a_inputId);

  void threadFunction();

  void startDispatchThread();
  void quitDispatchThread();

  void elementChanged(size_t a_id);

 private:
  bool tryDispatch();
  void dispatch(size_t a_id);

 private:
  std::vector<Element*> m_data{};
  std::vector<uint32_t> m_free{};

  moodycamel::ConcurrentQueue<size_t> m_queue{};
  spp::sparse_hash_map<size_t, std::set<size_t>> m_callbacks{};
  std::thread m_thread{};

  bool m_quit{};
};

} // namespace elements

#endif // ELEMENTS_PACKAGE_H