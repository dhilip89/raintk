/*
   Copyright (C) 2015 Preet Desai (preet.desai@gmail.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef RAINTK_COLUMN_HPP
#define RAINTK_COLUMN_HPP

#include <map>
#include <list>
#include <raintk/RainTkWidget.hpp>

namespace raintk
{
    class Column : public Widget
    {
    public:
        using base_type = raintk::Widget;

        Column(ks::Object::Key const &key,
               Scene* scene,
               shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<Column> const &);

        ~Column();

        void AddChild(shared_ptr<Widget> const &child) override;
        void RemoveChild(shared_ptr<Widget> const &child) override;

        // Properties
        Property<float> spacing{
            0.0f
        };

        Property<float> children_width{
            0.0f
        };

        Property<float> children_height{
            0.0f
        };

    protected:
        void onSpacingChanged();
        void onChildDimsChanged();

        Id m_cid_spacing;

    private:
        void update() override;

        struct Item
        {
            Widget* widget;
            Id cid_height;
            Id cid_width;
        };

        std::list<Item> m_list_items;
        std::map<Id,std::list<Item>::iterator> m_lkup_id_item_it;
    };
}

#endif // RAINTK_COLUMN_HPP
