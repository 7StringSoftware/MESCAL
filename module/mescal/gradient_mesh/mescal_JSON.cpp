
JSONStructure::JSONStructure()
{
}

JSONStructure::JSONStructure(juce::var jsonVar_) :
    jsonVar(jsonVar_)
{
}

void JSONStructure::toMemoryBlock(juce::MemoryBlock& block) const
{
    auto string = toString();
    auto bytesNeeded = juce::CharPointer_UTF8::getBytesRequiredFor(string.getCharPointer());
    if (bytesNeeded > block.getSize())
    {
        block.setSize(bytesNeeded + 1);
    }
    string.copyToUTF8((char*)block.getData(), block.getSize());
}

void JSONStructure::fromMemoryBlock(juce::MemoryBlock const& block)
{
    auto string = block.toString();
    jsonVar = juce::JSON::fromString(string);
}

juce::String JSONStructure::toString() const
{
    return juce::JSON::toString(jsonVar);
}

std::unique_ptr<JSONObject> JSONStructure::getObject() const
{
    return std::make_unique<JSONObject>(jsonVar);
}

std::unique_ptr<JSONArray> JSONStructure::getArray() const
{
    return std::make_unique<JSONArray>(jsonVar);
}

JSONObject::JSONObject(juce::var jsonVar_) :
    JSONStructure(jsonVar_)
{
}

JSONObject::JSONObject() :
    JSONStructure(juce::var{ new juce::DynamicObject })
{
}

JSONObject::JSONObject(const char* buffer, size_t bufferBytes)
{
    juce::String jsonString{ buffer, bufferBytes };
    auto result = juce::JSON::parse(jsonString, jsonVar);
#if JUCE_DEBUG
    if (result.failed())
    {
        DBG(result.getErrorMessage());
    }
#endif
    jassert(result.wasOk());
}

void JSONObject::fromMemoryBlock(juce::MemoryBlock const& block)
{
    JSONStructure::fromMemoryBlock(block);
}

#if JUCE_DEBUG
void JSONObject::dump(int depth /*= 0*/) const
{
    auto indent = juce::String::repeatedString(" ", depth * 3);
    if (auto object = jsonVar.getDynamicObject())
    {
        auto const& properties = object->getProperties();
        for (auto const& property : properties)
        {
            if (property.value.isArray())
            {
                DBG(indent << property.name << " : Array");

                JSONArray array{ property.value };
                array.dump(depth + 1);
                continue;
            }

            if (property.value.getDynamicObject() != nullptr)
            {
                DBG(indent << property.name << " : Object");

                JSONObject child{ property.value };
                child.dump(depth + 1);
                return;
            }

            if (property.value.isInt())
            {
                DBG(indent << property.name << "(int): " << (int)property.value);
            }
            else if (property.value.isInt64())
            {
                DBG(indent << property.name << "(int64): " << (int64_t)property.value);
            }
            else if (property.value.isBool())
            {
                DBG(indent << property.name << "(bool): " << (int)property.value);

            }
            else if (property.value.isDouble())
            {
                DBG(indent << property.name << "(double): " << (double)property.value);
            }
            else
            {
                DBG(indent << property.name << " : '" << property.value.toString() << "'");
            }
        }
    }
}
#endif

int JSONObject::getNumProperties() const
{
    if (auto object = jsonVar.getDynamicObject())
    {
        return object->getProperties().size();
    }

    return 0;
}

bool JSONObject::hasProperty(const juce::Identifier& propertyName) const
{
    if (auto object = jsonVar.getDynamicObject())
    {
        return object->hasProperty(propertyName);
    }

    return false;
}

template<> void JSONObject::set<JSONArray>(const juce::Identifier& propertyName, JSONArray array)
{
    set(propertyName, array.jsonVar);
}

void JSONObject::setArray(const juce::Identifier& propertyName, JSONArray array)
{
    set<JSONArray>(propertyName, array);
}

template<> JSONArray JSONObject::get<JSONArray>(const juce::Identifier& propertyName) const
{
    if (auto object = jsonVar.getDynamicObject())
    {
        return { object->getProperty(propertyName) };
    }

    return {};
}

JSONArray JSONObject::getArray(const juce::Identifier& propertyName) const
{
    return get<JSONArray>(propertyName);
}

JSONArray::JSONArray(juce::var jsonVar_) :
    JSONStructure(jsonVar_)
{
}

JSONArray::JSONArray() :
    JSONStructure(juce::Array<juce::var>{})
{
}

JSONArray::JSONArray(juce::StringArray const& strings_) :
    JSONStructure(juce::var{ strings_ })
{
}

void JSONArray::fromMemoryBlock(juce::MemoryBlock const& block)
{
    JSONStructure::fromMemoryBlock(block);
}

#if JUCE_DEBUG
void JSONArray::dump(int depth /*= 0*/) const
{
    auto indent = juce::String::repeatedString(" ", depth * 3);
    if (auto array = jsonVar.getArray())
    {
        for (int index = 0; index < array->size(); ++index)
        {
            auto const& entry = array->getReference(index);

            if (entry.isArray())
            {
                DBG(indent << index << " : Array");

                JSONArray childArray{ entry };
                childArray.dump(depth + 1);
                continue;
            }

            if (auto object = entry.getDynamicObject())
            {
                DBG(indent << index << " : Object");

                JSONObject childObject{ entry };
                childObject.dump(depth + 1);
                continue;
            }

            DBG(indent << index << " : '" << entry.toString() << "'");
        }
    }
}
#endif

int JSONArray::size() const
{
    if (auto array = jsonVar.getArray())
    {
        return array->size();
    }

    return 0;
}

#if JUCE_DEBUG
class JSONStructureTest : public juce::UnitTest
{
public:
    JSONStructureTest() : UnitTest("JSONStructureTest") {}

    void runTest() override
    {
        beginTest("Text to JSONStructure");

        juce::File jsonFile = juce::File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getChildFile("Common/test.json");
        juce::String jsonInputString = jsonFile.loadFileAsString();
        auto jsonVar = juce::JSON::parse(jsonInputString);
        auto goldenJSONString = juce::JSON::toString(jsonVar);
        juce::String checkJSONString;

        {
            JSONStructure root{ jsonVar };
            testGet(root);
        }

        {
            JSONObject root;

            root.set<juce::String>("SerialNumber", "1234567890");

            juce::MACAddress macAddress{ "BBBBBBBBBBBB" };
            root.set<juce::MACAddress>("MACAddress", macAddress);

            JSONArray addresses;
            addresses.add(juce::MACAddress{ "112233445566" });
            addresses.add(juce::MACAddress{ "778899AABBCC" });
            root.set<JSONArray>("PairedDevices", addresses);

            root.set<int32_t>("SelectedRadio", 1);

            root.set<double>("RSSI", -0.125);

            testGet(root);

            checkJSONString = juce::JSON::toString(root.jsonVar);
        }

        {
            JSONObject root;

            root.set<double>("Double", 1234.0);
            DBG(root.get<double>("Double"));
        }

        DBG("\ngoldenJSONString");
        DBG(goldenJSONString);
        DBG("\ncheckJSONString");
        DBG(checkJSONString);
        expect(goldenJSONString == checkJSONString);
    }

    void testGet(JSONStructure const& root)
    {
        auto rootObject = root.getObject();

        expect(rootObject.get() != nullptr);
        expect(rootObject->getNumProperties() == 5);

        auto serialString = rootObject->get<juce::String>("SerialNumber");
        expect(serialString == "1234567890");

        auto macAddress = rootObject->get<juce::MACAddress>("MACAddress");
        expect(macAddress == juce::MACAddress{ "BBBBBBBBBBBB" });

        //DBG("\nrootObject");
        //rootObject->dump();

        auto pairedDevices = rootObject->get<JSONArray>("PairedDevices");
        DBG("\npairedDevices");
        pairedDevices.dump();
        expect(pairedDevices.size() == 2);
        expect(pairedDevices.get<juce::MACAddress>(0) == juce::MACAddress{"112233445566"});
        expect(pairedDevices.get<juce::MACAddress>(1) == juce::MACAddress{"778899AABBCC"});

        int selectedRadio = rootObject->get<int>("SelectedRadio");
        expect(selectedRadio == 1);

        auto rssi = rootObject->get<double>("RSSI");
        expect(rssi == -0.125);
    }
};

static JSONStructureTest jsonStructureTest;
#endif
