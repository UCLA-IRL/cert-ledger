#include "mnemosyne/record.hpp"

#include <sstream>
#include <utility>
#include <iostream>

namespace mnemosyne {

Record::Record(Name recordName)
        : m_data(nullptr),
          m_recordName(std::move(recordName)) {
    for (int i = 0; i < m_recordName.size(); i ++) {
        if (readString(m_recordName.get(i)) == "RECORD") return;
        if (readString(m_recordName.get(i)) == "GENESIS_RECORD") return;
    }
    NDN_THROW(std::runtime_error("Bad record name"));
}

Record::Record(const Name &producerName, const Data &event)
        : m_data(nullptr),
          m_recordName(Name(producerName).append("RECORD").append(event.getName())) {
    setContentItem(event.wireEncode());
}

Record::Record(const std::shared_ptr<Data> &data)
        : m_data(data) {
    m_recordName = data->getName();
    headerWireDecode(m_data->getContent());
    if (!isGenesisRecord())
        bodyWireDecode(m_data->getContent());
}

Record::Record(ndn::Data data)
        : Record(std::make_shared<ndn::Data>(std::move(data))) {
}

Name
Record::getRecordName() const {
    if (m_data != nullptr)
        return m_data->getFullName();
    return Name();
}

const std::list<Name> &
Record::getPointersFromHeader() const {
    return m_recordPointers;
}

void
Record::setContentItem(const Block &contentItem) {
    if (m_data != nullptr) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot modify built record"));
    }
    m_contentItem = contentItem;
}

const Block &
Record::getContentItem() const {
    return m_contentItem;
}

bool
Record::isEmpty() const {
    return m_data == nullptr && m_recordPointers.empty() && !m_contentItem.isValid();
}

void
Record::addPointer(const Name &pointer) {
    if (m_data != nullptr) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot modify built record"));
    }
    m_recordPointers.push_back(pointer);
}

void
Record::wireEncode(Block &block) const {
    headerWireEncode(block);
    bodyWireEncode(block);
}

Name
Record::getProducerPrefix() const {
    for (int i = 0; i < m_recordName.size(); i ++) {
        if (readString(m_recordName.get(i)) == "RECORD" || readString(m_recordName.get(i)) == "GENESIS_RECORD")
            return m_recordName.getPrefix(i);
    }
    return Name();
}

void
Record::headerWireEncode(Block &block) const {
    auto header = makeEmptyBlock(T_RecordHeader);
    for (const auto &pointer : m_recordPointers) {
        header.push_back(pointer.wireEncode());
    }
    header.parse();
    block.push_back(header);
    block.parse();
}

void
Record::headerWireDecode(const Block &dataContent) {
    m_recordPointers.clear();
    dataContent.parse();
    const auto &headerBlock = dataContent.get(T_RecordHeader);
    headerBlock.parse();
    Name pointer;
    for (const auto &item : headerBlock.elements()) {
        if (item.type() == tlv::Name) {
            try {
                pointer.wireDecode(item);
            } catch (const tlv::Error &e) {
                std::cout << (e.what());
            }

            m_recordPointers.push_back(pointer);
        } else {
            BOOST_THROW_EXCEPTION(std::runtime_error("Bad header item type"));
        }
    }
}

void
Record::bodyWireEncode(Block &block) const {
    auto body = makeEmptyBlock(T_RecordContent);
    body.push_back(m_contentItem);
    body.parse();
    block.push_back(body);
    block.parse();
};

void
Record::bodyWireDecode(const Block &dataContent) {
    dataContent.parse();
    const auto &contentBlock = dataContent.get(T_RecordContent);
    m_contentItem = contentBlock.blockFromValue();
}

void
Record::checkPointerCount(int numPointers) const {
    if (getPointersFromHeader().size() != numPointers) {
        throw std::runtime_error("Less preceding record than expected");
    }

    std::set<Name> nameSet;
    for (const auto &pointer: getPointersFromHeader()) {
        nameSet.insert(pointer);
    }
    if (nameSet.size() != numPointers) {
        throw std::runtime_error("Repeated preceding Records");
    }
}

Name Record::getEventName() const {
    for (int i = 0; i < m_recordName.size(); i ++) {
        if (readString(m_recordName.get(i)) == "RECORD" || readString(m_recordName.get(i)) == "GENESIS_RECORD")
            return m_recordName.getSubName(i + 1);
    }
    return Name();
}

bool Record::isGenesisRecord() const {
    for (int i = 0; i < m_recordName.size(); i ++) {
        if (readString(m_recordName.get(i)) == "RECORD") return false;
        if (readString(m_recordName.get(i)) == "GENESIS_RECORD") return true;
    }
    return false;
}

GenesisRecord::GenesisRecord(int number) :
    Record(Name("/mnemosyne/GENESIS_RECORD/").append(std::to_string(number)))
{
    setContentItem(makeEmptyBlock(tlv::Name));
}

}  // namespace mnemosyne