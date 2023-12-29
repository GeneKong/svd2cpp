#pragma once

#include <fmt/core.h>
#include <vector>

class EAccess
{
public:
    enum Value
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    EAccess() : value(ReadWrite) {}
    EAccess(Value e) : value(e) {}

    const std::string toString() const
    {
        switch (value)
        {
        case ReadOnly:
            return "ReadOnly";
        case WriteOnly:
            return "WriteOnly";
        default:
            return "ReadWrite";
        }
    }

    bool operator==(const EAccess& other) const
    {
        return value == other.value;
    }

private:
    Value value;
};

class EnumUsage
{
  public:
    enum Value
    {
        Read,
        Write,
        ReadWrite
    };

    EnumUsage() : value(ReadWrite) {}
    EnumUsage(Value e) : value(e) {}

    const std::string toString() const
    {
        switch (value)
        {
            case Read:
                return "Read";
            case Write:
                return "Write";
            default:
                return "ReadWrite";
        }
    }

    bool operator==(const EnumUsage& other) const
    {
        return value == other.value;
    }

  private:
    Value value;
};

struct EnumValue {
    std::string name;
    std::string description;
    unsigned int value;
    void display() const
    {
        fmt::print(
            "\t\t\t\tname: {}\n"
            "\t\t\t\tdescription: {}\n"
            "\t\t\t\tvalue: {}\n",
            name, description, value);
    }
};

struct Enum
{
    std::string name;
    EnumUsage usage;
    std::vector< EnumValue > values;

    void display() const
    {
        fmt::print(
            "\t\tname: {}\n"
            "\t\tusage: {}\n"
            "\t\tvalues: \n",
            name, usage.toString());
        for( auto& i : values ) {
            i.display();
        }
    }
};

struct Field
{
    std::string name;
    std::string description;
    unsigned int bitOffset;
    unsigned int bitWidth;
    EAccess fieldAccess;
    std::vector< Enum > enums;

    // Dim
    unsigned int dim;
    unsigned int dimIncrement;
    std::vector<std::string> dimIndex;

    void display() const
    {
        fmt::print(
            "\t\tname: {}\n"
            "\t\tdescription: {}\n"
            "\t\tbitOffset: {}\n"
            "\t\tbitWidth: {}\n"
            "\t\tfieldAccess: {}\n"
            "\t\tenums: \n",
            name, description, bitOffset, bitWidth, fieldAccess.toString());
        for( auto& i : enums ) {
            i.display();
        }
    }
};

struct Register
{
    std::string name;
    std::string displayName;
    std::string description;
    unsigned int addressOffset;
    unsigned int size;
    EAccess registerAccess;
    unsigned int resetValue;
    std::vector< Field > fields;

    // Dim
    unsigned int dim;
    unsigned int dimIncrement;
    std::vector<std::string> dimIndex;

    // Cluster Dim fields below
    std::string dimName;
    std::string dimDescription;
    unsigned int dimAddressOffset;
    // Register in cluster
    std::vector<Register> registers;

    void display() const
    {
        fmt::print(
            "\tname: {}\n"
            "\tdescription: {}\n"
            "\taddressOffset: {:x}\n"
            "\tsize: {}\n"
            "\tregisterAccess: {}\n"
            "\tresetValue: {}\n"
            "\tfields: \n\n",
            name, description, addressOffset, size, registerAccess.toString(), resetValue);
        for( auto& i : fields ) {
            i.display();
        }
    }
};

struct AddressBlock
{
    unsigned int offset;
    unsigned int size;
    void display() const
    {
        fmt::print(
            "\toffset: {:x}\n"
            "\tsize: {:x}\n",
            offset, size
            );
    }
};

struct Peripheral
{
    std::string name;
    std::string description;
    std::string groupName;
    unsigned int baseAddress;
    AddressBlock addressBlock;
    std::vector< Register > registers;
    void display() const
    {
        fmt::print(
            "name: {}\n"
            "description: {}\n"
            "groupName: {}\n"
            "baseAddress: 0x{:08x}\n"
            "addressBlock: \n",
            name, description, groupName, baseAddress);
        addressBlock.display();
        fmt::print("registers: \n");
        for( auto& i : registers ) {
            i.display();
        }
    }
};
