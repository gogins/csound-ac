/*
* C S O U N D A C
*
* Copyright (c) 2001-2003 by Michael Gogins. All rights reserved.
*
* CsoundAC is a Python extension module for doing algorithmic
* composition, in one which one writes music by programming in
* Python. Musical events are points in music space with dimensions
* {time, duration, event type, instrument, pitch as MIDI key,
* loudness as MIDI velocity, phase, pan, depth, height, pitch-class
* set, 1}, and pieces are composed by assembling a hierarchical tree
* of nodes in music space. Each node has its own local transformation
* of coordinates in music space. Nodes can be empty, contain scores
* or fragments of scores, generate scores, or transform
* scores. CsoundAC also contains a Python interface to the Csound
* API, making it easy to render CsoundAC compositions using Csound.
*
* L I C E N S E
*
* This software is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma SWIG nowarn=401
#pragma SWIG nowarn=362
#pragma SWIG nowarn=503

#if defined(SWIGPYTHON)

%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

/* This function already gets exported in other form */
/* Don't export this to avoid build failures on amd64 */
%ignore Counterpoint::message(const char*, va_list);
%ignore csound::print(const char*, va_list);

%module(directors="1") CsoundAC
%{
    #include <algorithm>
    #include <cstring>
    #include "Silence.hpp"
%}
%apply int { size_t };

/* Python 3.12+: PyString_* are gone; use str (Unicode) + heap copy for each element. */
%typemap(in) char ** {
  if (!PyList_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "not a list");
    return NULL;
  }
  const Py_ssize_t size = PyList_GET_SIZE($input);
  $1 = (char **) malloc((size + 1) * sizeof(char *));
  if (!$1) {
    PyErr_NoMemory();
    return NULL;
  }
  for (Py_ssize_t i = 0; i < size; ++i) {
    PyObject *o = PyList_GET_ITEM($input, i);
    if (!o || !PyUnicode_Check(o)) {
      PyErr_SetString(PyExc_TypeError, "list must contain str");
      for (Py_ssize_t j = 0; j < i; ++j) free($1[j]);
      free($1);
      $1 = NULL;
      return NULL;
    }
    const char *utf8 = PyUnicode_AsUTF8(o);
    if (!utf8) {
      for (Py_ssize_t j = 0; j < i; ++j) free($1[j]);
      free($1);
      $1 = NULL;
      return NULL;
    }
    const size_t len = std::strlen(utf8);
    $1[i] = (char *) malloc(len + 1u);
    if (!$1[i]) {
      for (Py_ssize_t j = 0; j < i; ++j) free($1[j]);
      free($1);
      $1 = NULL;
      PyErr_NoMemory();
      return NULL;
    }
    std::memcpy($1[i], utf8, len + 1u);
  }
  $1[size] = NULL;
}

%typemap(freearg) char ** {
  if ($1) {
    for (char **p = $1; *p; ++p) {
      free(*p);
    }
    free($1);
  }
}

/* Directors need the full Python C API; Py_LIMITED_API (stable ABI across minors) is not a
   practical combination with SWIG directors—prefer separate builds per Python minor (e.g. wheels). */
%feature("director") Node;
%include <Silence.hpp>
%include <Conversions.hpp>
%include <System.hpp>

%template(vectorScale) std::vector<csound::Scale>;
%template(vectorChord) std::vector<csound::Chord>;
%template(vectorString) std::vector<std::string>;

#endif
