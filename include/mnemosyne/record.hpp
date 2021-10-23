#ifndef MNEMOSYNE_INCLUDE_RECORD_H_
#define MNEMOSYNE_INCLUDE_RECORD_H_

#include <set>
#include <vector>
#include <list>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/security/certificate.hpp>

using namespace ndn;
namespace mnemosyne {

/**
 * The record.
 * Record Name: /<producer-prefix>/RECORD/<event-name>
 */
class Record {
  public: // used for preparing a new record before appending it into the DLedger
    Record() = default;

    /**
     * Construct a new record.
     * @p identifier, input, the unique identifer of the record.
     */
    Record(Name recordName);
    Record(const Name &producerName, const Data &eventItem);

    /**
     * Add a new record payload item into the record.
     * @note This function should only be used to generate a record before adding it to the ledger.
     * @p recordItem, input, the record payload to add.
     */
    void
    setContentItem(const Block &contentItem);

    /**
     * Get the NDN Data full name of the record.
     * This name is not the identifier used in the constructor of the record.
     * The name is only generated when adding the record into the DLedger.
     * @note This function should only be used to parse a record returned from the ledger.
     *       This cannot be used when a record has not been appended into the ledger
     * @p recordItem, input, the record payload to add.
     */
    Name
    getRecordName() const;

    /**
      * Get the record unique identifier of the record.
      */
    Name
    getEventName() const;

    /**
     * Get record payload.
     */
    const Block &
    getContentItem() const;

    /**
     * Check whether the record body is empty or not.
     */
    bool
    isEmpty() const;

  public: // used for generating a new record before appending it into the Mnemosyne
    /**
     * @note This constructor is supposed to be used by the Mnemosyne class only
     */
    Record(const std::shared_ptr<Data> &data);

    /**
     * @note This constructor is supposed to be used by the Mnemosyne class only
     */
    Record(Data data);

    /**
     * Get the pointers from the header.
     * @note This function is supposed to be used by the DLedger class only
     */
    const std::list<Name> &
    getPointersFromHeader() const;

    /**
     * Add new pointers to the header.
     * @note This function is supposed to be used by the DLedger class only
     */
    void
    addPointer(const Name &pointer);

    /**
     * validate the pointers in a header.
     * @param numPointers number of pointer according to the config
     * @note This function is supposed to be used by the DLedger class only
     */
    void
    checkPointerCount(int numPointers) const;

    /**
     * Encode the record header and body into the block.
     * @p block, output, the Data Content block to carry the encoded record.
     */
    void
    wireEncode(Block &block) const;

    Name
    getProducerPrefix() const;

    bool
    isGenesisRecord() const;

    /**
     * Data packet with name
     * /<application-common-prefix>/<producer-name>/<record-type>/<record-name>/<timestamp>
     */
    std::shared_ptr<const Data> m_data;

  private:
    void
    headerWireEncode(Block &block) const;

    void
    bodyWireEncode(Block &block) const;

    void
    headerWireDecode(const Block &dataContent);

    void
    bodyWireDecode(const Block &dataContent);

  private:
    /**
     * The TLV type of the record header in the NDN Data Content.
     */
    const static uint8_t T_RecordHeader = 129;
    /**
     * The TLV type of the record body in the NDN Data Content.
     */
    const static uint8_t T_RecordContent = 130;

  private:
    /**
     * The record-name as
     * /<producer-prefix>/RECORD/<event-name>
     */
    Name m_recordName;

  protected:
    /**
     * The list of pointers to preceding records.
     */
    std::list<Name> m_recordPointers;
    /**
     * The data structure to carry the record body payloads.
     */
    Block m_contentItem;

    friend class Mnemosyne;
};

// name: /mnemosyne/GENESIS_RECORD/number
class GenesisRecord : public Record {
  public:
    GenesisRecord(int number);
};

} // namespace mnemosyne

#endif // define MNEMOSYNE_INCLUDE_RECORD_H_

