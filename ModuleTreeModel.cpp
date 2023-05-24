/**
 * \date 2021
 * \copyright Allied Vision Technologies.  All Rights Reserved.
 *
 * \copyright Redistribution of this file, in original or modified form, without
 *            prior written consent of Allied Vision Technologies is prohibited.
 *
 * \warning THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
 * NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * \brief Implementation of ::VmbC::Examples::ModuleTreeModel
 */

#include "ModuleTreeModel.h"

#include <limits>
#include <unordered_map>
/* 实现左侧设备树的树形结构 */
namespace VmbC
{
    namespace Examples
    {
        ModuleTreeModel::Item::Item()
            : m_parent(nullptr),
            m_indexInParent((std::numeric_limits<size_t>::max)())
           /* 这个构造函数用于创建一个空的 Item 对象，没有关联的 ModuleData 对象。 */ 
        {
        }

        ModuleTreeModel::Item::Item(std::unique_ptr<ModuleData>&& module)
            : m_module(std::move(module)), m_parent(nullptr),
            m_indexInParent((std::numeric_limits<size_t>::max)())
            /* 接受一个右值引用的 std::unique_ptr<ModuleData> 参数 module，并将其移动到成员变量 m_module 中。
            同时，初始化 m_parent 为 nullptr，m_indexInParent 为 std::numeric_limits<size_t>::max()。
            这个构造函数用于创建一个带有指定的 ModuleData 对象的 Item 对象。 */
        {
        }

        ModuleTreeModel::ModuleTreeModel(std::vector<std::unique_ptr<ModuleData>>&& moduleData)
        /*brief： 
        构造函数接受一个右值引用的 std::vector<std::unique_ptr<ModuleData>> 参数 moduleData，并根据该参数初始化模型的数据。
        首先，通过 reserve 分配足够的内存空间，然后使用移动语义将 moduleData 中的元素转移至 m_data。
        接下来，创建一个映射表 mapping，将每个 ModuleData 对象与包含它的 Item 对象进行关联。
        然后，初始化每个 Item 对象的父项和子项列表，设置相应的父项和索引值。 */
        {
            m_data.reserve(moduleData.size());

            for (auto& md : moduleData)
            {
                m_data.emplace_back(std::move(md));
            }
            m_data.shrink_to_fit();
            
            // create mapping for ModuleData to Item containing it
            std::unordered_map<ModuleData*, Item*> mapping;
            mapping[nullptr] = &m_pseudoRoot;

            for (auto& item : m_data)
            {
                mapping[item.m_module.get()] = &item;
            }

            // initialize parent and child lists
            for (auto& item : m_data)
            {
                auto parentItem = mapping[item.m_module->GetParent()];

                item.m_parent = parentItem;
                item.m_indexInParent = parentItem->m_children.size();
                parentItem->m_children.push_back(&item);
            }
        }

        QModelIndex ModuleTreeModel::index(int r, int column, QModelIndex const& parent) const
        /* 根据给定的行号、列号和父索引，返回相应的模型索引。
        如果列号不为0或行号小于0，则返回一个无效的模型索引。
        如果存在有效的父索引，则根据父项的子项列表返回相应的索引；
        否则，在根项的子项列表中返回相应的索引 */
        {
            if (column != 0 || r < 0)
            {
                return QModelIndex();
            }

            size_t const row = static_cast<size_t>(r);

            if (parent.isValid())
            {
                auto const ptr = parent.internalPointer();
                if (column != 0 || ptr == nullptr)
                {
                    return QModelIndex();
                }
                auto& children = static_cast<Item*>(ptr)->m_children;

                return (children.size() > row)
                    ? createIndex(r, column, children[row]) : QModelIndex();
            }
            else
            {
                return row >= m_pseudoRoot.m_children.size() ? QModelIndex() : createIndex(r, column, m_pseudoRoot.m_children[row]);
            }
        }

        QModelIndex ModuleTreeModel::parent(QModelIndex const& index) const
        /* 返回给定索引的父索引。如果索引无效，则返回一个无效的模型索引。
        如果存在有效的父项，则根据父项的索引和0列号创建相应的索引并返回；
        否则返回一个无效的模型索引。 */
        {
            if (index.isValid())
            {
                Item const* const item = static_cast<Item*>(index.internalPointer());
                if (item == nullptr)
                {
                    return QModelIndex();
                }
                else
                {
                    auto const parentItem = item->m_parent;
                    return (parentItem == nullptr) ? QModelIndex() : createIndex(static_cast<int>(parentItem->m_indexInParent), 0, parentItem);
                }
            }
            else
            {
                return QModelIndex();
            }
        }

        int ModuleTreeModel::rowCount(QModelIndex const& parent) const
        /* 返回给定父索引下的子项数量。如果存在有效的父索引，则返回父项的子项数量；否则返回根项的子项数量。 */
        {
            if (parent.isValid())
            {
                Item const* const item = static_cast<Item*>(parent.internalPointer());
                return (item == nullptr) ? 0 : static_cast<int>(item->m_children.size());
            }
            else
            {
                return static_cast<int>(m_pseudoRoot.m_children.size());
            }
        }

        int ModuleTreeModel::columnCount(QModelIndex const& parent) const
        /* 返回列数，这里始终为1。 */
        {
            return 1;
        }

        namespace
        
        {
            struct DataRetrievalContext
            /* brief：该结构体用于在数据检索过程中传递上下文信息，以便在访问器中存储数据的结果。
            m_role：表示数据检索的角色（Qt::ItemDataRole类型）。
            m_result：表示数据检索的结果（QVariant类型的引用）。
             */
            {
                Qt::ItemDataRole m_role;
                QVariant& m_result;
            };
        }

        QVariant ModuleTreeModel::data(QModelIndex const& index, int role) const
        /* 返回给定索引和角色下的数据。
        首先创建一个空的 QVariant 对象 result。
        然后获取索引对应的 Item 对象，并使用 DataRetrievalVisitor 访问器从 Item 对象的关联 ModuleData 中获取数据并存入 result 中，使用指定的角色。
        最后返回 result。 */
        {
            QVariant result;

            Item const* const item = static_cast<Item*>(index.internalPointer());

            if (item != nullptr)
            {
                DataRetrievalVisitor visitor { static_cast<Qt::ItemDataRole>(role), result };
                item->m_module->Accept(visitor);
            }

            return result;
        }

        Qt::ItemFlags ModuleTreeModel::flags(QModelIndex const& index) const
        /* 返回给定索引下的项标志。首先将标志设置为 Qt::ItemIsEnabled。
        如果索引有效，则获取索引对应的 Item 对象，并使用 FlagUpdateVisitor 访问器更新标志，根据关联的 ModuleData 对象的属性。
        最后返回更新后的标志。 */
        {
            Qt::ItemFlags flags = Qt::ItemFlag::ItemIsEnabled;

            if (index.isValid())
            {
                auto ptr = index.internalPointer();
                if (ptr != nullptr)
                {
                    FlagUpdateVisitor visitor(flags);
                    static_cast<Item*>(ptr)->m_module->Accept(visitor);
                }
            }

            return flags;
        }

        ModuleData const* ModuleTreeModel::GetModule(QModelIndex const& modelIndex)
        /* 根据给定的模型索引返回关联的 ModuleData 对象指针。
        如果模型索引无效，则返回 nullptr。
        否则，获取索引对应的 Item 对象，并返回其关联的 ModuleData 对象指针。 */
        {
            if (!modelIndex.isValid())
            {
                return nullptr;
            }
            auto ptr = modelIndex.internalPointer();
            return (ptr == nullptr) ? nullptr : static_cast<Item*>(ptr)->m_module.get();
        }

        void ModuleTreeModel::DataRetrievalVisitor::Visit(VmbCameraInfo_t const& data)
        /* 于在数据检索过程中访问不同类型的数据并根据角色设置数据结果。

        对于VmbCameraInfo_t类型的数据，根据角色的不同，执行不同的操作：

        当角色为Qt::ItemDataRole::DisplayRole时，将data.modelName和data.cameraName组合成一个字符串作为结果。
         */
        {
            switch (m_role)
            {
            case Qt::ItemDataRole::DisplayRole:
                m_result = QString(data.modelName) + QString(" (") + QString(data.cameraName) + QString(")");
                break;
            }
        }

        void ModuleTreeModel::DataRetrievalVisitor::Visit(VmbInterfaceInfo_t const& data)
        /* 根据角色的不同，执行不同的操作：
        当角色为Qt::ItemDataRole::DisplayRole时，将data.interfaceName作为结果。 */
        {
            switch (m_role)
            {
            case Qt::ItemDataRole::DisplayRole:
                m_result = QString(data.interfaceName);
                break;
            }
        }

        void ModuleTreeModel::DataRetrievalVisitor::Visit(VmbTransportLayerInfo_t const& data)
        /* 
        根据角色的不同，执行不同的操作：
        当角色为Qt::ItemDataRole::DisplayRole时，将data.transportLayerName作为结果。
        当角色为Qt::ItemDataRole::ToolTipRole时，将data.transportLayerName加上前缀信息作为结果。 */
        {
            switch (m_role)
            {
            case Qt::ItemDataRole::DisplayRole:
                m_result = QString(data.transportLayerName);
                break;
            case Qt::ItemDataRole::ToolTipRole:
                m_result = QString::fromStdString(std::string("transportLayerName: ") + data.transportLayerName);
                break;
            }
        }

        ModuleTreeModel::DataRetrievalVisitor::DataRetrievalVisitor(Qt::ItemDataRole role, QVariant& result)
            : m_role(role), m_result(result)
            /* 构造函数接受一个Qt::ItemDataRole类型的角色参数和一个对QVariant类型的结果的引用参数。
            在构造函数中，这两个参数被用于初始化m_role和m_result成员变量。 */
        {
        }

        void ModuleTreeModel::FlagUpdateVisitor::Visit(VmbCameraInfo_t const& data)
        /* 对于VmbCameraInfo_t类型的数据，访问器将根据数据的特性设置相应的标志。在这种情况下，设置的标志有：

        Qt::ItemFlag::ItemNeverHasChildren：表示该项永远没有子项。
        Qt::ItemFlag::ItemIsSelectable：表示该项可以被选中。
        这样，当访问器访问到VmbCameraInfo_t类型的数据时，将根据数据特性更新m_flags，最终决定了项的标志。 */
        {
            m_flags |= (Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsSelectable);
        }
} // namespace Examples
} // namespace VmbC
