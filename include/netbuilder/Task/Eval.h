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

#ifndef NETBUILDER__TASK__EVAL_H
#define NETBUILDER__TASK__EVAL_H

#include "netbuilder/Types.h"

#include "netbuilder/Task/BaseTask.h"
#include "netbuilder/FigureOfMerit/FigureOfMerit.h"

#include <boost/signals2.hpp>

#include <memory>
#include <limits>

namespace NetBuilder { namespace Task {

class Eval : public BaseTask 
{
    public:

        Eval(std::unique_ptr<DigitalNet> net, std::unique_ptr<FigureOfMerit::FigureOfMerit> figure):
            m_net(std::move(net)),
            m_merit(0),
            m_figure(std::move(figure))
        {};

        Eval(Eval&&) = default;

        ~Eval() = default;

        /**
        * Returns the dimension.
        */
        Dimension dimension() const
        { return m_net->dimension(); }

        unsigned int nRows() {return m_net->numRows(); }

        unsigned int nCols() {return m_net->numColumns(); }

        /**
        * Returns the best net found by the search task.
        */
        const DigitalNet& net() const
        { return *m_net; }

        /**
        * Returns the best net found by the search task.
        */
        virtual const DigitalNet& netOutput() const
        { return net(); }

        /**
        * Returns the best merit value found by the search task.
        */
        Real meritValue() const
        { return m_merit; }

        /**
        * Returns the best merit value found by the search task.
        */
        virtual Real meritValueOutput() const
        { return meritValue(); }

        const FigureOfMerit::FigureOfMerit& figureOfMerit() const 
        {
               return *m_figure;
        }

        /**
        * Executes the search task.
        *
        * The best net and merit value are set in the process.
        */
        virtual void execute() {

            auto evaluator = m_figure->evaluator();

            m_merit = evaluator->operator()(*m_net, m_merit);
        }

        virtual void reset()
        {
            m_merit = 0;
        }

    private:

        std::unique_ptr<DigitalNet> m_net;
        Real m_merit;
        std::unique_ptr<FigureOfMerit::FigureOfMerit> m_figure;
        unsigned int m_verbose = 1;

};


}}

#endif
