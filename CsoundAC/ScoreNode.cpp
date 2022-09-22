/*
 * C S O U N D
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
#include "ScoreNode.hpp"

namespace csound
{
ScoreNode::ScoreNode() : duration(0.0)
{
}

ScoreNode::~ScoreNode()
{
}

void ScoreNode::generate(Score &collectingScore)
{
    if(importFilename.length() > 0) {
        score.std::vector<Event>::clear();
        score.load(importFilename);
    }
    score.sort();
    if (duration != 0.0) {
        score.setDuration(duration);
    }
    auto header = score.getCsoundScoreHeader();
    if (header.empty() == false) {
        collectingScore.appendToCsoundScoreHeader(header);
        collectingScore.appendToCsoundScoreHeader("\n");
    }
    for (int i = 0, n = score.size(); i < n; ++i) {
        collectingScore.push_back(score[i]);
        // TODO fix this hack... much work!
        collectingScore.back().process = score[i].process;
    }
}

Score &ScoreNode::getScore()
{
    return score;
}
}
