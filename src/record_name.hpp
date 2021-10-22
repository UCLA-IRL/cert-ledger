//
// Created by Tyler on 6/3/20.
//

#ifndef MNEMOSYNE_RECORD_NAME_HPP
#define MNEMOSYNE_RECORD_NAME_HPP

#include <ndn-cxx/name.hpp>
#include <mnemosyne/config.hpp>
#include <mnemosyne/record.hpp>

using namespace ndn;
namespace mnemosyne {

class RecordName : public Name {
  public:
    // record Name: /<producer-prefix>/<record-type>/<record-name>/<timestamp>
    RecordName(const Name &name);

    RecordName(const Name &peerPrefix, RecordType type, const std::string &identifier,
               time::system_clock::TimePoint time = time::system_clock::now());

    Name getProducerPrefix() const;

    RecordType getRecordType() const;

    std::string getRecordUniqueIdentifier() const;

    time::system_clock::TimePoint getGenerationTimestamp() const;

    bool hasImplicitDigest() const;

    std::string getImplicitDigest() const;

    //generate names
    static RecordName generateRecordName(const Config &config, const Record &record);
};

}


#endif //MNEMOSYNE_RECORD_NAME_HPP
