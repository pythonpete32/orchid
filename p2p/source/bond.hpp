/* Orchid - WebRTC P2P VPN Market (on Ethereum)
 * Copyright (C) 2017-2019  The Orchid Authors
*/

/* GNU Affero General Public License, Version 3 {{{ */
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/* }}} */


#ifndef ORCHID_BOND_HPP
#define ORCHID_BOND_HPP

#include <set>

#include "link.hpp"

namespace orc {

class Bonded {
  private:
    class Bonding :
        public Pipe,
        public BufferDrain
    {
      private:
        Bonded *bonded_;

      protected:
        virtual Pump *Inner() = 0;

        void Land(const Buffer &data) override {
            return bonded_->Land(this, data);
        }

        void Stop(const std::string &error) override {
            bonded_->bondings_.erase(this);
        }

      public:
        Bonding(Bonded *bonded) :
            bonded_(bonded)
        {
        }

        task<void> Send(const Buffer &data) override {
            co_return co_await Inner()->Send(data);
        }
    };

    std::set<Bonding *> bondings_;

  protected:
    virtual void Land(Pipe *pipe, const Buffer &data) = 0;

    Sunk<> *Wire() {
        // XXX: this is clearly incorrect
        auto bonding(new Sink<Bonding>(this));
        auto backup(bonding);
        bondings_.emplace(std::move(bonding));
        return backup;
    }

  public:
    task<void> Send(const Buffer &data) {
        auto bonding(bondings_.begin());
        if (bonding == bondings_.end())
            co_return;
        co_return co_await (*bonding)->Send(data);
    }
};

}

#endif//ORCHID_BOND_HPP