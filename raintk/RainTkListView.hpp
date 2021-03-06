/*
   Copyright (C) 2016 Preet Desai (preet.desai@gmail.com)

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

#include <raintk/RainTkListModelSTLVector.hpp>
#include <raintk/RainTkListDelegate.hpp>
#include <raintk/RainTkScrollArea.hpp>
#include <raintk/RainTkScene.hpp>

#ifdef RAINTK_DEBUG_LIST_VIEW_GUIDELINES
    #include <raintk/RainTkRectangle.hpp>
#endif

namespace raintk
{
    // =========================================================== //

    struct ListViewProperties
    {
        enum class Layout
        {
            // Row, // TODO
            Column
        };

        // TODO
//        enum class Order
//        {
//            Ascending,
//            Descending
//        };
    };

    // =========================================================== //

    class ListViewDelegateHeightInvalid : public ks::Exception
    {
    public:
        ListViewDelegateHeightInvalid(std::string msg);

        ~ListViewDelegateHeightInvalid() = default;
    };

    // =========================================================== //

    template<typename ItemType, typename DelegateType>
    class ListView : public raintk::ScrollArea
    {
        using ListViewType = ListView<ItemType,DelegateType>;

        using ListDelegates =
            std::vector<shared_ptr<DelegateType>>;

        class ContentPosition
        {
        public:
            ContentPosition(Property<float>* property,
                            std::function<void(float)> setter) :
                m_property(property),
                m_setter(std::move(setter))
            {}

            void Assign(float val)
            {
                m_setter(val);
            }

            float Get()
            {
                return m_property->Get();
            }

        private:
            Property<float>* m_property;
            std::function<void(float)> m_setter;
        };

        shared_ptr<ListModel<ItemType>> m_list_model;

        // * Connections
        Id m_cid_content_x_changed;
        Id m_cid_content_y_changed;
        Id m_cid_before_adding_items;
        Id m_cid_added_items;
        Id m_cid_before_removing_items;
        Id m_cid_removed_items;
        Id m_cid_data_changed;
        Id m_cid_layout_changed;

        // * Delegate bounds
        Property<float> m_delegate_bounds_top{
            0.0f
        };

        Property<float> m_delegate_bounds_bottom{
            0.0f
        };

        Property<float> m_delegate_bounds_left{
            0.0f
        };

        Property<float> m_delegate_bounds_right{
            0.0f
        };

        // These properties point to the four above properties
        // depending on the layout direction and order
        Property<float>* m_delegate_bounds_start;
        Property<float>* m_delegate_bounds_end;

        Property<float>* m_view_size;
        Property<float>* m_content_size;
        unique_ptr<ContentPosition> m_content_position;


        // * The average height or width of a delegate, used
        //   to estimate the dimensions of the content_parent
        //   and place the first delegate
        float m_average_delegate_size{0};
        float m_minimum_delegate_size{mm(1.0f)}; // TODO setter/getter
        uint m_max_delegate_count{30}; // TODO setter/getter

        std::function<float(shared_ptr<DelegateType> const &)> m_get_delegate_size;
        std::function<float(shared_ptr<DelegateType> const &)> m_get_delegate_position;
        std::function<void(shared_ptr<DelegateType> const &,float)> m_set_delegate_position;

        ListDelegates m_list_delegates;

        bool m_upd_reposition;

#ifdef RAINTK_DEBUG_LIST_VIEW_GUIDELINES
        // * Guidelines to help debug, should be disabled
        //   in release builds
        using ListRectangles = std::vector<shared_ptr<Rectangle>>;
        ListRectangles m_list_guidelines;
#endif


    public:
        // Properties
        Property<float> delegate_extents{
            mm(25)
        };

        Property<float> spacing{
            0.0f
        };

        // TODO Should this be a template parameter instead?
        Property<ListViewProperties::Layout> layout{
            ListViewProperties::Layout::Column
        };

        // === //

        using base_type = raintk::ScrollArea;

        ListView(ks::Object::Key const &key,
                 Scene* scene,
                 shared_ptr<Widget> parent) :
            raintk::ScrollArea(key,scene,parent),
            m_upd_reposition(false)
        {}

        ~ListView()
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<ListViewType> const &this_view)
        {
            setupLayoutDirection();

            // Setup connections

            // Setup a slot for both content x and y changed
            // signals so we don't have to change them when
            // the layout direction changes -- the ScrollArea
            // should lock respective directions and prevent
            // unused signals anyway
            m_cid_content_y_changed =
                    m_content_parent->y.signal_changed.Connect(
                        this_view,
                        &ListViewType::onScrollPositionChanged,
                        ks::ConnectionType::Direct);

            m_cid_content_x_changed =
                    m_content_parent->x.signal_changed.Connect(
                        this_view,
                        &ListViewType::onScrollPositionChanged,
                        ks::ConnectionType::Direct);

            layout.signal_changed.Connect(
                        this_view,
                        &ListViewType::onLayoutDirectionChanged,
                        ks::ConnectionType::Direct);
        }

        void SetListModel(shared_ptr<ListModel<ItemType>> list_model)
        {
            // Remove all previous delegates
            for(auto& delegate : m_list_delegates)
            {
                this->RemoveChild(delegate);
            }

            m_list_delegates.clear();

            // Disconnect previous connections
            if(m_list_model)
            {
                m_list_model->signal_before_adding_items.Disconnect(
                            m_cid_before_adding_items);

                m_list_model->signal_added_items.Disconnect(
                            m_cid_added_items);

                m_list_model->signal_before_removing_items.Disconnect(
                            m_cid_before_removing_items);

                m_list_model->signal_removed_items.Disconnect(
                            m_cid_removed_items);

                m_list_model->signal_data_changed.Disconnect(
                            m_cid_data_changed);

                m_list_model->signal_layout_changed.Disconnect(
                            m_cid_layout_changed);
            }

            // Setup new connections
            m_list_model = list_model;

            shared_ptr<ListViewType> this_view =
                    std::static_pointer_cast<ListViewType>(
                        shared_from_this());

            m_cid_before_adding_items =
                    m_list_model->signal_before_adding_items.Connect(
                        this_view,
                        &ListViewType::onBeforeAddingItems,
                        ks::ConnectionType::Direct);

            m_cid_added_items = m_list_model->signal_added_items.Connect(
                        this_view,
                        &ListViewType::onAddedItems,
                        ks::ConnectionType::Direct);

            m_cid_before_removing_items =
                    m_list_model->signal_before_removing_items.Connect(
                        this_view,
                        &ListViewType::onBeforeRemovingItems,
                        ks::ConnectionType::Direct);

            m_cid_removed_items =
                    m_list_model->signal_removed_items.Connect(
                        this_view,
                        &ListViewType::onRemovedItems,
                        ks::ConnectionType::Direct);

            m_cid_data_changed =
                    m_list_model->signal_data_changed.Connect(
                        this_view,
                        &ListViewType::onDataChanged,
                        ks::ConnectionType::Direct);

            m_cid_layout_changed =
                    m_list_model->signal_layout_changed.Connect(
                        this_view,
                        &ListViewType::onLayoutChanged,
                        ks::ConnectionType::Direct);


            m_content_parent->x = 0;
            m_content_parent->y = 0;
            m_content_parent->width = width.Get();
            m_content_parent->height = height.Get();

            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;
        }

#ifdef RAINTK_DEBUG_LIST_VIEW_GUIDELINES
        void ShowDebugGuidelines(bool show)
        {
            if(show)
            {
                createDebugGuidelines();
            }
            else
            {
                for(auto guideline : m_list_guidelines)
                {
                    guideline->GetParent()->RemoveChild(guideline);
                }
                m_list_guidelines.clear();
            }
        }
#endif

    private:
        void onBeforeAddingItems(uint idx_before,
                                 uint count)
        {
            (void)idx_before;
            (void)count;
        }

        void onAddedItems(uint idx_first_added,
                          uint idx_end_added)
        {
            // Update the widget (at the very least, the content
            // parent size has changed)
            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;


            // If the current delegate list is empty or the
            // newly added range is after the current delegates
            if(m_list_delegates.empty() ||
              (idx_first_added > m_list_delegates.back()->GetIndex()))
            {
                // Nothing else needs to be done
                return;
            }

            uint idx_last_added = idx_end_added-1;


            // If the added range is before the current delegates
            if(idx_last_added < m_list_delegates.front()->GetIndex())
            {
                // Push the content position and delegates by an
                // estimate of the space taken up by the new items
                float shift =
                        (idx_end_added-idx_first_added)*
                        (m_average_delegate_size+spacing.Get());

                float prev_content_position =
                        m_content_position->Get();

                m_content_position->Assign(
                            prev_content_position-shift);

                // Update positions and indices
                uint added_count = idx_end_added-idx_first_added;
                for(auto& delegate : m_list_delegates)
                {
                    auto old_pos = m_get_delegate_position(delegate);
                    m_set_delegate_position(delegate,old_pos+shift);

                    delegate->SetIndex(
                                delegate->GetIndex()+
                                added_count);
                }

                return;
            }

            // The newly inserted range intersects with the current
            // range of delegates
            auto it_insert = getDelegateItForModelIndexNoCheck(idx_first_added);

            float const spacing_val = spacing.Get();
            float delegate_position = m_get_delegate_position((*it_insert));
            float total_shift = 0;

            for(uint i=idx_first_added; i < idx_end_added; i++)
            {
                auto delegate = createDelegate(i);
                m_set_delegate_position(delegate,delegate_position);

                float shift =
                        (m_get_delegate_size(delegate))+
                        spacing_val;

                delegate_position += shift;
                total_shift += shift;

                it_insert =
                        m_list_delegates.insert(
                            it_insert,
                            delegate);

                ++it_insert;
            }

            // Update the indices and shift positions for
            // delegates after the inserted range
            auto it_delegate = getDelegateItForModelIndexNoCheck(idx_end_added);

            uint model_index = idx_end_added;
            while(it_delegate != m_list_delegates.end())
            {
                auto& delegate = *it_delegate;
                delegate->SetIndex(model_index);

                auto old_position =
                        m_get_delegate_position(delegate);

                m_set_delegate_position(
                            delegate,old_position+total_shift);

                model_index++;
                ++it_delegate;
            }
        }

        void onBeforeRemovingItems(uint idx_first_remove,
                                   uint idx_end_remove)
        {
            (void)idx_first_remove;
            (void)idx_end_remove;

            // Update the widget (at the very least, the content
            // parent size has changed)
            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;

            // If the current delegate list is empty or the
            // range to be removed is after the current delegates
            if(m_list_delegates.empty() ||
              (idx_first_remove > m_list_delegates.back()->GetIndex()))
            {
                // Nothing else needs to be done
                return;
            }

            uint idx_last_remove = idx_end_remove-1;
            uint removed_count = idx_end_remove-idx_first_remove;

            // If the range to be removed is before the current delegates
            if(idx_last_remove < m_list_delegates.front()->GetIndex())
            {
                // Push the content position and delegates by an
                // estimate of the space taken up by the removed items
                float shift =
                        (removed_count)*
                        (m_average_delegate_size+spacing.Get());

                float prev_content_position =
                        m_content_position->Get();

                m_content_position->Assign(
                            prev_content_position+shift);

                // Update positions and indices
                for(auto& delegate : m_list_delegates)
                {
                    auto old_pos = m_get_delegate_position(delegate);
                    m_set_delegate_position(delegate,old_pos-shift);

                    delegate->SetIndex(
                                delegate->GetIndex()-
                                removed_count);
                }

                return;
            }

            // If the range to be removed intersects with the current
            // range of delegates

            // Get list of delegates to remove and measure the
            // resulting shift
            float total_shift=0.0f;
            float spacing_val = spacing.Get();

            std::vector<typename ListDelegates::iterator> list_rem_its(removed_count);

            auto it_remove = getDelegateItForModelIndexNoCheck(idx_first_remove);

            for(uint i=0; i < removed_count; i++)
            {
                list_rem_its[removed_count-1-i] = it_remove;
                total_shift += (spacing_val + m_get_delegate_size(*it_remove));
                ++it_remove;
            }

            // Update positions and indices of delegates following
            // the range to be removed
            while(it_remove != m_list_delegates.end())
            {
                auto& delegate = (*it_remove);
                delegate->SetIndex(delegate->GetIndex()-removed_count);

                auto old_pos = m_get_delegate_position(delegate);
                m_set_delegate_position(delegate,old_pos-total_shift);

                ++it_remove;
            }

            // Remove delegates (do it this way to be more efficient)
            for(auto it : list_rem_its)
            {
                m_content_parent->RemoveChild(*it);
                m_list_delegates.erase(it);
            }
        }

        void onRemovedItems(uint idx_after_removed,
                            uint count)
        {
            (void)idx_after_removed;
            (void)count;
        }

        void onDelegateDimChanged()
        {
            m_upd_reposition = true;

            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;
        }

        void onDataChanged(uint model_index)
        {
            auto it_update = getDelegateItForModelIndex(model_index);
            if(it_update != m_list_delegates.end())
            {
                (*it_update)->SetData(m_list_model->GetData(model_index));
            }
        }

        void onLayoutChanged()
        {
            m_list_delegates.clear();
            m_content_parent->x = 0;
            m_content_parent->y = 0;
            m_content_parent->width = width.Get();
            m_content_parent->height = height.Get();

            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;
        }

        typename std::vector<shared_ptr<DelegateType>>::iterator
        getDelegateItForModelIndex(uint model_index)
        {
            if(!m_list_delegates.empty())
            {
                bool outside_range =
                        model_index < m_list_delegates.front()->GetIndex() ||
                        model_index > m_list_delegates.back()->GetIndex();

                if(!outside_range)
                {
                    return std::next(
                                m_list_delegates.begin(),
                                model_index-m_list_delegates.front()->GetIndex());
                }
            }

            return m_list_delegates.end();
        }

        typename std::vector<shared_ptr<DelegateType>>::iterator
        getDelegateItForModelIndexNoCheck(uint model_index)
        {
            return std::next(
                        m_list_delegates.begin(),
                        model_index-m_list_delegates.front()->GetIndex());
        }

        void setupLayoutDirection()
        {
            if(layout.Get()==ListViewProperties::Layout::Column)
            {
                m_view_size = &(height);
                m_content_size = &(m_content_parent->height);

                m_content_position =
                        make_unique<ContentPosition>(
                            &(m_content_parent->y),
                            [this](float val){ this->SetContentY(val); });

                m_delegate_bounds_start = &(m_delegate_bounds_top);
                m_delegate_bounds_end = &(m_delegate_bounds_bottom);

                m_get_delegate_size =
                        [](shared_ptr<DelegateType> const &d) -> float {
                            return d->height.Get();
                        };

                m_get_delegate_position =
                        [](shared_ptr<DelegateType> const &d) -> float {
                            return d->y.Get();
                        };

                m_set_delegate_position =
                        [](shared_ptr<DelegateType> const &d, float new_position) {
                            return d->y = new_position;
                        };
            }
            else
            {
                m_view_size = &(width);
                m_content_size = &(m_content_parent->width);

                m_content_position =
                        make_unique<ContentPosition>(
                            &(m_content_parent->x),
                            [this](float val){ this->SetContentX(val); });

                m_delegate_bounds_start = &(m_delegate_bounds_left);
                m_delegate_bounds_end = &(m_delegate_bounds_right);

                m_get_delegate_size =
                        [](shared_ptr<DelegateType> const &d) -> float {
                            return d->width.Get();
                        };

                m_get_delegate_position =
                        [](shared_ptr<DelegateType> const &d) -> float {
                            return d->x.Get();
                        };

                m_set_delegate_position =
                        [](shared_ptr<DelegateType> const &d, float new_position) {
                            return d->x = new_position;
                        };
            }

            calcDelegateBounds();
        }

        void onLayoutDirectionChanged()
        {
            setupLayoutDirection();
        }

        void onScrollPositionChanged()
        {
            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;
        }

        // Fix the delegate positions after their dimensions have changed
        // The delegates are shifted relative to the first delegate
        void repositionDelegates()
        {
            float spacing_val = spacing.Get();

            float position =
                    m_get_delegate_position(m_list_delegates.front())+
                    m_get_delegate_size(m_list_delegates.front())+
                    spacing_val;

            auto it_delegate = std::next(m_list_delegates.begin());
            while(it_delegate != m_list_delegates.end())
            {
                auto& delegate = *it_delegate;
                m_set_delegate_position(delegate,position);

                position += (m_get_delegate_size(delegate)+spacing_val);
                ++it_delegate;
            }
        }

        void update()
        {
            if(m_list_model==nullptr || m_list_model->GetSize()==0)
            {
                return;
            }

            if(m_upd_reposition)
            {
                repositionDelegates();
                m_upd_reposition = false;
            }

            // Erase all delegates outside of the delegate extents
            eraseDelegatesOutsideExtents(
                        m_delegate_bounds_top.Get(),
                        m_delegate_bounds_bottom.Get(),
                        m_delegate_bounds_left.Get(),
                        m_delegate_bounds_right.Get());

            if(m_list_delegates.empty())
            {
                // Create the first delegate using an estimated
                // index based on the current position of the list

                calcAverageDelegateSize();
                updateContentParentSize();

                // Get model index based on content position
                float estimated_model_index =
                        (m_content_position->Get()*-1.0f)/
                        (m_average_delegate_size+spacing.Get());

                // Create delegate for model index
                m_list_delegates.push_back(
                            createDelegate(estimated_model_index));

                m_set_delegate_position(
                            m_list_delegates.back(),
                            m_content_position->Get());
            }

            // There must be at least one delegate at this point
            fillSpaceBefore();
            fillSpaceAfter();
            correctDelegatePositions();
            calcAverageDelegateSize();
            updateContentParentSize();
        }

        shared_ptr<DelegateType> createDelegate(uint model_index)
        {
            auto delegate = MakeWidget<DelegateType>(m_scene,m_content_parent);
            delegate->SetData(m_list_model->GetData(model_index));
            delegate->SetIndex(model_index);
            delegate->UpdateHierarchy();

            // NOTE:
            // Avoiding using the shared_ptr for this object for
            // lifetime tracking to avoid creating one everytime;
            // should be okay
            if(layout.Get() == ListViewProperties::Layout::Column)
            {
                delegate->m_cid_delegate_height =
                        delegate->height.signal_changed.Connect(
                            this,
                            &ListViewType::onDelegateDimChanged,
                            nullptr,
                            ks::ConnectionType::Direct);
            }
            else
            {
                delegate->m_cid_delegate_width =
                        delegate->width.signal_changed.Connect(
                            this,
                            &ListViewType::onDelegateDimChanged,
                            nullptr,
                            ks::ConnectionType::Direct);
            }

            return delegate;
        }

        // Fill available space before the first delegate
        // in list_delegates
        void fillSpaceBefore()
        {
            sint model_index = m_list_delegates.front()->GetIndex();
            model_index--;

            // ie. The edge closest towards the start
            // of the delegate bounds
            float first_start_position =
                    m_get_delegate_position(
                        m_list_delegates.front());

            float space_before =
                    first_start_position-m_delegate_bounds_start->Get();

            while((space_before > 0) &&
                  (model_index >= 0) &&
                  (m_list_delegates.size() <= m_max_delegate_count))
            {
                // Ensure that adding another delegate would place
                // it at least partially within the delegate bounds
                if(space_before-spacing.Get() <= 0.0f)
                {
                    break;
                }

                auto delegate = createDelegate(model_index);

                m_list_delegates.insert(
                            m_list_delegates.begin(),
                            delegate);

                first_start_position -= (spacing.Get()+m_get_delegate_size(delegate));
                m_set_delegate_position(delegate,first_start_position);
                space_before = first_start_position-m_delegate_bounds_start->Get();

                model_index--;
            }
        }

        // Fill available space after the last delegate
        // in list_delegates
        void fillSpaceAfter()
        {
            uint model_index = m_list_delegates.back()->GetIndex();
            model_index++;

            // ie. The edge closest towards the end of
            // the delegate bounds
            float last_end_position =
                    m_get_delegate_position(
                        m_list_delegates.back())+
                    m_get_delegate_size(
                        m_list_delegates.back());

            float space_after =
                    m_delegate_bounds_end->Get()-last_end_position;

            while((space_after > 0) &&
                  (model_index < m_list_model->GetSize()) &&
                  (m_list_delegates.size() <= m_max_delegate_count))
            {
                // Ensure that adding another delegate would place
                // it at least partially within the delegate bounds
                if(space_after-spacing.Get() <= 0.0f)
                {
                    break;
                }

                auto delegate = createDelegate(model_index);

                m_list_delegates.push_back(delegate);

                last_end_position += spacing.Get();
                m_set_delegate_position(delegate,last_end_position);
                last_end_position += m_get_delegate_size(delegate);
                space_after = m_delegate_bounds_end->Get()-last_end_position;

                model_index++;
            }
        }

        void correctDelegatePositions()
        {
            auto& first_delegate = m_list_delegates.front();
            bool require_shift = false;

            if(first_delegate->GetIndex()==0)
            {
                require_shift = (fabs(m_get_delegate_position(first_delegate)) > 1E-1);
            }
            else
            {
                // If content position 0.0f is within the delegate bounds
                // but the first delegate isn't index 0
                require_shift = (m_delegate_bounds_start->Get() < 0.0f);
            }

            if(require_shift)
            {
                // Shift all the delegates along to the correct position
                float position_shift =
                        m_get_delegate_position(
                            m_list_delegates.front())*-1.0f;

                for(auto& delegate : m_list_delegates)
                {
                    float new_position =
                            m_get_delegate_position(delegate)+
                            position_shift;

                    m_set_delegate_position(
                                delegate,new_position);
                }

                // Shift the current position of the content_parent so
                // that the view doesn't 'jump'
                m_content_position->Assign(
                            m_content_position->Get()+
                            (position_shift*-1.0f));
            }
        }

        void updateContentParentSize()
        {
            float estimated_size =
                    (m_average_delegate_size+spacing.Get())*
                     m_list_model->GetSize();

            estimated_size -= spacing.Get();

            if(m_list_delegates.empty())
            {
                m_content_size->Assign(estimated_size);
                return;
            }

            auto& last_delegate = m_list_delegates.back();

            float last_delegate_end =
                    m_get_delegate_position(last_delegate)+
                    m_get_delegate_size(last_delegate);

            if(last_delegate->GetIndex() ==
                    m_list_model->GetSize()-1)
            {
                if(m_content_size->Get() != last_delegate_end)
                {
                    m_content_size->Assign(last_delegate_end);
                }
            }
            else
            {
                auto new_size =
                        std::max(last_delegate_end,
                                 estimated_size);

                if(m_content_size->Get() != new_size)
                {
                    m_content_size->Assign(new_size);
                }
            }
        }

        void calcDelegateBounds()
        {
            if(layout.Get() == ListViewProperties::Layout::Column)
            {
                m_delegate_bounds_top =
                        [this](){
                            return -1.0f*m_content_parent->y.Get()-
                                delegate_extents.Get();
                        };

                m_delegate_bounds_bottom =
                        [this](){
                            return m_delegate_bounds_top.Get()+
                                    height.Get()+
                                    2*delegate_extents.Get();
                        };

                m_delegate_bounds_left =
                        [this](){
                            return m_content_parent->x.Get();
                        };

                m_delegate_bounds_right =
                        [this](){
                            return m_delegate_bounds_left.Get()+
                                    width.Get();
                        };
            }
        }

        void calcAverageDelegateSize()
        {
            if(m_list_model->GetSize() == 0)
            {
                return;
            }

            m_average_delegate_size=0;

            if(m_list_delegates.size() > 0)
            {
                for(auto& delegate : m_list_delegates)
                {
                    m_average_delegate_size += m_get_delegate_size(delegate);
                }
                m_average_delegate_size /= (float)m_list_delegates.size();
            }
            else
            {
                // Instantiate some delegates
                // TODO is 5 enough?

                uint const delegate_count =
                        std::min<uint>(5,m_list_model->GetSize());

                for(uint i=0; i < delegate_count; i++)
                {
                    auto delegate = createDelegate(i);

                    m_average_delegate_size += m_get_delegate_size(delegate);
                    m_content_parent->RemoveChild(delegate);
                }

                m_average_delegate_size /= (float)delegate_count;
            }

            // Enforce a minimum height
            if(m_average_delegate_size < mm(1))
            {
                m_average_delegate_size = mm(1);
            }
        }

        void eraseDelegatesOutsideExtents(float vext_t,
                                          float vext_b,
                                          float vext_l,
                                          float vext_r)
        {
            uint remove_count=0;
            for(auto it = m_list_delegates.end();
                it != m_list_delegates.begin();)
            {
                --it;

                auto& delegate = *it;

                // TODO fix
                float delegate_t = delegate->y.Get();
                float delegate_b = delegate_t + delegate->height.Get();
                float delegate_l = delegate->x.Get();
                float delegate_r = delegate_l + delegate->width.Get();

                bool outside_vext =
                        (delegate_b < vext_t) ||
                        (delegate_t > vext_b) ||
                        (delegate_r < vext_l) ||
                        (delegate_l > vext_r);

                if(outside_vext)
                {
                    remove_count++;
                    m_content_parent->RemoveChild(delegate);
                    it = m_list_delegates.erase(it);
                }
            }
        }

#ifdef RAINTK_DEBUG_LIST_VIEW_GUIDELINES
        void createDebugGuidelines()
        {
            if(layout.Get() == ListViewProperties::Layout::Column)
            {
                auto& lgd = m_list_guidelines;

                for(auto guideline : lgd)
                {
                    guideline->GetParent()->RemoveChild(guideline);
                }

                lgd.clear();

                // Create guidelines for the delegate extents window

                // top
                lgd.push_back(
                            MakeWidget<Rectangle>(
                                m_scene,
                                m_content_parent));

                lgd.back()->name = ks::ToString(GetId())+"guideline_delegates_top";
                lgd.back()->width = [this](){ return width.Get(); };
                lgd.back()->height = mm(0.25f);
                lgd.back()->y = [this](){ return m_delegate_bounds_top.Get(); };
                lgd.back()->z = mm(20.0f);

                // bottom
                lgd.push_back(
                            MakeWidget<Rectangle>(
                                m_scene,
                                m_content_parent));

                lgd.back()->name = ks::ToString(GetId())+"guideline_delegates_bottom";
                lgd.back()->width = [this](){ return width.Get(); };
                lgd.back()->height = mm(0.25f);
                lgd.back()->y = [this](){ return m_delegate_bounds_bottom.Get(); };
                lgd.back()->z = mm(20.0f);


                // Create guidelines for the list view content edges

                // top
                lgd.push_back(
                            MakeWidget<Rectangle>(
                                m_scene,
                                m_content_parent));

                lgd.back()->name = ks::ToString(GetId())+"guideline_content_top";
                lgd.back()->width = [this](){ return width.Get(); };
                lgd.back()->height = mm(0.25f);
                lgd.back()->y = mm(0.0f);
                lgd.back()->z = mm(20.0f);
                lgd.back()->color = glm::u8vec4{255,0,255,255};

                // bottom
                lgd.push_back(
                            MakeWidget<Rectangle>(
                                m_scene,
                                m_content_parent));

                lgd.back()->name = ks::ToString(GetId())+"guideline_content_bottom";
                lgd.back()->width = [this](){ return width.Get(); };
                lgd.back()->height = mm(0.25f);
                lgd.back()->y = [this](){ return m_content_parent->height.Get()-mm(0.25f); };
                lgd.back()->z = mm(20.0f);
                lgd.back()->color = glm::u8vec4{255,0,255,255};
            }
        }
#endif

    };

    // =========================================================== //
}
