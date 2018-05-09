// This file is part of Lattice Builder.
//
// Copyright (C) 2012-2016  Pierre L'Ecuyer and Universite de Montreal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef NETBUILDER__PARSER__NET_CONSTRUCTION_PARSER_H
#define NETBUILDER__PARSER__NET_CONSTRUCTION_PARSER_H

#include "latbuilder/Parser/Common.h"
#include "netbuilder/Types.h"

namespace NetBuilder { namespace Parser {
namespace lbp = LatBuilder::Parser;
/**
 * Exception thrown when trying to parse an invalid size parameter.
 */
class BadNetConstruction : public lbp::ParserError {
public:
   BadNetConstruction(const std::string& message):
      lbp::ParserError("cannot parse construction parameter string: " + message)
   {}
};

/**
 * Parser for construction parameters.
 */
struct NetConstructionParser {
   typedef NetBuilder::NetConstruction result_type;

   static result_type parse(const std::string& str)
   {
      if (str == "sobol")
         return NetBuilder::NetConstruction::SOBOL;
    //   else if (str == "polynomial")
    //      return NetBuilder::NetConstruction::POLYNOMIAL;
      throw BadNetConstruction(str);
   }
};

}}

#endif
