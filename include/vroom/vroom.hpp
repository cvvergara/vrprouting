/*PGR-GNU*****************************************************************
File: vroom.hpp

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
Copyright (c) 2021 Ashish Kumar
Mail: ashishkr23438@gmail.com
------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/

#ifndef INCLUDE_VROOM_VROOM_HPP_
#define INCLUDE_VROOM_VROOM_HPP_
#pragma once

#include <structures/vroom/input/input.h>
#include <structures/vroom/job.h>
#include <structures/vroom/vehicle.h>

#include <utility>
#include <vector>

#include "c_types/vroom_rt.h"
#include "cpp_common/vroom_types.hpp"
#include "cpp_common/base_matrix.hpp"
#include "cpp_common/messages.hpp"

namespace vrprouting {

class Vrp_vroom_problem : public vrprouting::Messages {
 public:
     void add_matrix(vrprouting::base::Base_Matrix);
     void add_jobs(const std::vector<Vroom_job_t>&, const std::vector<Vroom_time_window_t>&);
     void add_shipments(
             const std::vector<Vroom_shipment_t>&,
             const std::vector<Vroom_time_window_t>&);
     void add_vehicles(
             const std::vector<Vroom_vehicle_t>&,
             const std::vector<Vroom_break_t>&,
             const std::vector<Vroom_time_window_t>&);
     std::vector<Vroom_rt> solve(int32_t, int32_t, int32_t);

 private:
     std::vector<vroom::Job> jobs() const { return m_jobs; }
     std::vector<std::pair<vroom::Job, vroom::Job>> shipments() const { return m_shipments; }
     std::vector<vroom::Vehicle> vehicles() const { return m_vehicles; }
     vrprouting::base::Base_Matrix matrix() const { return m_matrix; }

     /** @name vroom time window wrapper */
     /** @{ */
     vroom::TimeWindow get_vroom_time_window(const Vroom_time_window_t&) const;
     vroom::TimeWindow get_vroom_time_window(Duration, Duration) const;
     std::vector<vroom::TimeWindow> get_vroom_time_windows(const std::vector<Vroom_time_window_t>&) const;
     /** @} */


     /** @name vroom amounts wrapper */
     /** @{ */
     vroom::Amount get_vroom_amounts(const std::vector<Amount> &) const;
     /** @} */


     /**
      * @name vroom jobs wrapper
      */
     /** @{ */
     /** @brief Gets the vroom jobs.  */
     vroom::Job get_vroom_job(const Vroom_job_t&, const std::vector<Vroom_time_window_t>&) const;
     void problem_add_job(const Vroom_job_t&, const std::vector<Vroom_time_window_t>&);
     /** @} */


     /** @name vroom shipments wrapper */
     /** @{ */
     /** @brief Gets the vroom shipments.  */
     std::pair<vroom::Job, vroom::Job> get_vroom_shipment(
             const Vroom_shipment_t&,
             const std::vector<Vroom_time_window_t>&,
             const std::vector<Vroom_time_window_t>&) const;

     void problem_add_shipment(
             const Vroom_shipment_t&,
             const std::vector<Vroom_time_window_t>&,
             const std::vector<Vroom_time_window_t>&);
     /** @} */


     /** @name vroom breaks wrapper */
     /** @{ */
     vroom::Break get_vroom_break(
             const Vroom_break_t&,
             const std::vector<Vroom_time_window_t>&) const;

     std::vector<vroom::Break> get_vroom_breaks(
             const std::vector<Vroom_break_t>&,
             const std::vector<Vroom_time_window_t>&) const;


     /** @name vroom vehicles wrapper */
     /** @{ */
     vroom::Vehicle get_vroom_vehicle(
             const Vroom_vehicle_t&,
             const std::vector<Vroom_break_t>&,
             const std::vector<Vroom_time_window_t>&) const;

     void problem_add_vehicle(
             const Vroom_vehicle_t&,
             const std::vector<Vroom_break_t>&,
             const std::vector<Vroom_time_window_t>&);
     /** @} */

     void get_amount(vroom::Amount, Amount**);
     StepType get_job_step_type(vroom::JOB_TYPE);
     StepType get_step_type(vroom::Step);

     std::vector<Vroom_rt> get_results(vroom::Solution);

 private:
     std::vector<vroom::Job> m_jobs;
     std::vector<std::pair<vroom::Job, vroom::Job>> m_shipments;
     std::vector<vroom::Vehicle> m_vehicles;
     vrprouting::base::Base_Matrix m_matrix;
};

}  // namespace vrprouting

#endif  // INCLUDE_VROOM_VROOM_HPP_
