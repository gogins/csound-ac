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
#include "CppSound.hpp"
#include "Sequence.hpp"
#include "System.hpp"

namespace csound
{

Sequence::Sequence()
{
}

Sequence::~Sequence()
{
}

void Sequence::traverse(const Eigen::MatrixXd &globalCoordinates,
                        Score &collectingScore)
{
    Eigen::MatrixXd compositeCoordinates = getLocalCoordinates() * globalCoordinates;
    // Child node times are rescaled to start from 0, and the next child node 
    // starts at the prior child node's last off time.
    Score child_score;

    double earliest_on_time = 0.0;
    double latest_off_time = 0.0;
    for (size_t child_i = 0, child_n = children.size(); child_i < child_n; ++child_i) {
        child_score.clear();
        children[child_i]->traverse(compositeCoordinates, child_score);
        earliest_on_time = child_score.getFirstOnTime();
        System::message("Sequence node at time %f: child %d of %d has %d notes.\n", earliest_on_time, child_i, child_n, child_score.size());
        for(size_t event_i = 0, event_n = child_score.size(); event_i < event_n; ++event_i) {
            Event event(child_score.at(event_i));
            event.setTime(event.getTime() - earliest_on_time + latest_off_time);
            System::inform("    %s\n", event.toString().c_str());
            collectingScore.push_back(event);
        }
        latest_off_time = latest_off_time + child_score.getLastOffTime();
    }
}

}
