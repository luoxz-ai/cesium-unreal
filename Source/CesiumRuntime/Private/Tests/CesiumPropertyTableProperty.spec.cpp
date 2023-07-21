#include "CesiumGltfSpecUtility.h"
#include "CesiumPropertyArrayBlueprintLibrary.h"
#include "CesiumPropertyTableProperty.h"
#include "Misc/AutomationTest.h"
#include <limits>

using namespace CesiumGltf;

BEGIN_DEFINE_SPEC(
    FCesiumPropertyTablePropertySpec,
    "Cesium.PropertyTableProperty",
    EAutomationTestFlags::ApplicationContextMask |
        EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FCesiumPropertyTablePropertySpec)

void FCesiumPropertyTablePropertySpec::Define() {
  Describe("Constructor", [this]() {
    It("constructs invalid instance by default", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          0);

      FCesiumMetadataValueType expectedType; // Invalid type
      TestTrue(
          "ValueType",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetValueType(
              property) == expectedType);
    });

    It("constructs invalid instance from view with invalid definition",
       [this]() {
         PropertyTablePropertyView<int8_t> propertyView(
             PropertyTablePropertyViewStatus::ErrorArrayTypeMismatch);
         FCesiumPropertyTableProperty property(propertyView);
         TestEqual(
             "PropertyTablePropertyStatus",
             UCesiumPropertyTablePropertyBlueprintLibrary::
                 GetPropertyTablePropertyStatus(property),
             ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
         TestEqual(
             "Size",
             UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
                 property),
             0);

         FCesiumMetadataValueType expectedType; // Invalid type
         TestTrue(
             "ValueType",
             UCesiumPropertyTablePropertyBlueprintLibrary::GetValueType(
                 property) == expectedType);
       });

    It("constructs invalid instance from view with invalid data", [this]() {
      PropertyTablePropertyView<int8_t> propertyView(
          PropertyTablePropertyViewStatus::ErrorBufferViewOutOfBounds);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidPropertyData);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          0);

      FCesiumMetadataValueType expectedType; // Invalid type
      TestTrue(
          "ValueType",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetValueType(
              property) == expectedType);
    });

    It("constructs valid instance", [this]() {
      std::vector<int32_t> values{1, 2, 3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);
      PropertyTablePropertyView<int32_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);

      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64_t>(values.size()));

      FCesiumMetadataValueType expectedType(
          ECesiumMetadataType::Scalar,
          ECesiumMetadataComponentType::Int32,
          false);
      TestTrue(
          "ValueType",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetValueType(
              property) == expectedType);
      TestEqual(
          "BlueprintType",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetBlueprintType(
              property),
          ECesiumMetadataBlueprintType::Integer);

      // Test that the returns are as expected for non-array properties.
      TestEqual(
          "ArraySize",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArraySize(property),
          0);
      TestEqual(
          "ArrayElementBlueprintType",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetArrayElementBlueprintType(property),
          ECesiumMetadataBlueprintType::None);
    });

    It("constructs instance for fixed-length array property", [this]() {
      const int64_t arrayCount = 3;
      std::vector<int32_t> values{1, 2, 3, 4, 5, 6};
      const int64_t size = static_cast<int64_t>(
          values.size() / static_cast<int64_t>(arrayCount));
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<PropertyArrayView<int32_t>> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(),
          gsl::span<const std::byte>(),
          PropertyComponentType::None,
          PropertyComponentType::None,
          arrayCount,
          size,
          false);

      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64_t>(size));

      FCesiumMetadataValueType expectedType(
          ECesiumMetadataType::Scalar,
          ECesiumMetadataComponentType::Int32,
          true);
      TestTrue(
          "ValueType",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetValueType(
              property) == expectedType);
      TestEqual(
          "BlueprintType",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetBlueprintType(
              property),
          ECesiumMetadataBlueprintType::Array);

      TestEqual(
          "ArraySize",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArraySize(property),
          arrayCount);
      TestEqual(
          "ArrayElementBlueprintType",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetArrayElementBlueprintType(property),
          ECesiumMetadataBlueprintType::Integer);
    });

    It("constructs instance for variable-length array property", [this]() {
      std::vector<int32_t> values{1, 2, 3, 4, 5, 6};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      std::vector<uint16_t> offsets{
          0,
          1 * sizeof(int32_t),
          3 * sizeof(int32_t),
          6 * sizeof(int32_t)};
      std::vector<std::byte> offsetsData = GetValuesAsBytes(offsets);

      int64_t size = static_cast<int64_t>(offsets.size()) - 1;
      PropertyTablePropertyView<PropertyArrayView<int32_t>> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(offsetsData.data(), offsetsData.size()),
          gsl::span<const std::byte>(),
          PropertyComponentType::Uint16,
          PropertyComponentType::None,
          0,
          size,
          false);

      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          size);

      FCesiumMetadataValueType expectedType(
          ECesiumMetadataType::Scalar,
          ECesiumMetadataComponentType::Int32,
          true);
      TestTrue(
          "ValueType",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetValueType(
              property) == expectedType);
      TestEqual(
          "BlueprintType",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetBlueprintType(
              property),
          ECesiumMetadataBlueprintType::Array);

      // The arrays vary in length, so GetArraySize() should return zero.
      TestEqual(
          "ArraySize",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArraySize(property),
          0);
      TestEqual(
          "ArrayElementBlueprintType",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetArrayElementBlueprintType(property),
          ECesiumMetadataBlueprintType::Integer);
    });
  });

  Describe("GetBoolean", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestFalse(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetBoolean(
              property,
              0));
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<std::byte> data{static_cast<std::byte>(0b10110001)};

      PropertyTablePropertyView<bool> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(8),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(8));

      TestFalse(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetBoolean(
              property,
              -1));
      TestFalse(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetBoolean(
              property,
              10));
    });

    It("gets from boolean property", [this]() {
      std::vector<std::byte> data{static_cast<std::byte>(0b10110001)};

      PropertyTablePropertyView<bool> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(8),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);

      std::vector<bool>
          expected{true, false, false, false, true, true, false, true};
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(expected.size()));

      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetBoolean(
                property,
                static_cast<int64>(i),
                false),
            expected[i]);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<std::string>
          values{"yes", "false", "invalid value", "true", "yeah", "1"};
      size_t totalSize = 0;
      for (const auto& value : values) {
        totalSize += value.size();
      }

      std::vector<std::byte> data(totalSize);
      std::vector<uint32_t> offsets(values.size() + 1);
      uint32_t currentOffset = 0;
      for (size_t i = 0; i < values.size(); ++i) {
        std::memcpy(
            data.data() + currentOffset,
            values[i].data(),
            values[i].size());
        offsets[i] = currentOffset;
        currentOffset += static_cast<uint32_t>(values[i].size());
      }
      offsets[offsets.size() - 1] = currentOffset;
      std::vector<std::byte> offsetsData = GetValuesAsBytes(offsets);

      PropertyTablePropertyView<std::string_view> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(),
          gsl::span<const std::byte>(offsetsData.data(), offsetsData.size()),
          PropertyComponentType::None,
          PropertyComponentType::Uint32,
          0,
          static_cast<int64_t>(values.size()),
          false);

      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<bool> expected{true, false, false, true, false, true};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetBoolean(
                property,
                static_cast<int64>(i),
                false),
            expected[i]);
      }
    });
  });

  Describe("GetByte", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetByte(property, 0),
          0);
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<uint8_t> values{1, 2, 3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<uint8_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetByte(property, -1),
          0);
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetByte(property, 10),
          0);
    });

    It("gets from uint8 property", [this]() {
      std::vector<uint8_t> values{1, 2, 3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<uint8_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetByte(
                property,
                static_cast<int64>(i),
                0),
            values[i]);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<int32_t> values{1, 24, 255, 256, -1, 28};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<int32_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<uint8_t> expected{1, 24, 255, 0, 0, 28};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetByte(
                property,
                static_cast<int64>(i),
                0),
            expected[i]);
      }
    });
  });

  Describe("GetInteger", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger(property, 0),
          0);
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<int32_t> values{-1, 2, -3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<int32_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger(
              property,
              -1),
          0);
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger(
              property,
              10),
          0);
    });

    It("gets from int32 property", [this]() {
      std::vector<int32_t> values{-1, 2, -3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<int32_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger(
                property,
                static_cast<int64>(i),
                0),
            values[i]);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<float> values{
          1.234f,
          -24.5f,
          std::numeric_limits<float>::lowest(),
          2456.80f,
          std::numeric_limits<float>::max(),
      };
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<float> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<int32_t> expected{1, -24, 0, 2456, 0};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger(
                property,
                static_cast<int64>(i),
                0),
            expected[i]);
      }
    });
  });

  Describe("GetInteger64", [this]() {
    int64_t defaultValue = static_cast<int64_t>(0);
    It("returns default value for invalid property", [this, defaultValue]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual<int64>(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger64(
              property,
              0),
          defaultValue);
    });

    It("returns default value for invalid feature ID", [this, defaultValue]() {
      std::vector<int64_t> values{-1, 2, -3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<int64_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual<int64>(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger64(
              property,
              -1),
          defaultValue);
      TestEqual<int64>(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger64(
              property,
              10),
          defaultValue);
    });

    It("gets from int64 property", [this, defaultValue]() {
      std::vector<int64_t> values{-1, 2, -3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<int64_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        TestEqual<int64>(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger64(
                property,
                static_cast<int64>(i),
                defaultValue),
            values[i]);
      }
    });

    It("converts compatible values", [this, defaultValue]() {
      std::vector<uint64_t> values{
          10,
          20,
          30,
          static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 100};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<uint64_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<int64_t> expected{10, 20, 30, 0};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual<int64>(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetInteger64(
                property,
                static_cast<int64>(i),
                defaultValue),
            expected[i]);
      }
    });
  });

  Describe("GetFloat", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat(
              property,
              0.0f),
          0.0f);
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<float> values{-1.1f, 2.2f, -3.3f, 4.0f};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<float> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat(property, -1),
          0.0f);
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat(property, 10),
          0.0f);
    });

    It("gets from float property", [this]() {
      std::vector<float> values{-1.1f, 2.2f, -3.3f, 4.0f};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<float> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat(
                property,
                static_cast<int64>(i),
                0),
            values[i]);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<double> values{
          -1.1,
          2.2,
          -3.3,
          std::numeric_limits<double>::max()};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<double> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<float> expected(4);
      for (size_t i = 0; i < values.size(); i++) {
        expected[i] = static_cast<float>(values[i]);
      }
      expected[3] = 0.0f;

      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat(
                property,
                static_cast<int64>(i),
                0.0f),
            expected[i]);
      }
    });
  });

  Describe("GetFloat64", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat64(
              property,
              0.0),
          0.0);
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<double> values{-1.1, 2.2, -3.3, 4.0};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<double> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat64(
              property,
              -1),
          0.0);
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat64(
              property,
              10),
          0.0);
    });

    It("gets from double property", [this]() {
      std::vector<double> values{-1.1, 2.2, -3.3, 4.0};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<double> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat64(
                property,
                static_cast<int64>(i),
                0),
            values[i]);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<std::string> values{"not a number", "10", "-2"};
      size_t totalSize = 0;
      for (const auto& value : values) {
        totalSize += value.size();
      }

      std::vector<std::byte> data(totalSize);
      std::vector<uint8_t> offsets(values.size() + 1);
      uint8_t currentOffset = 0;
      for (size_t i = 0; i < values.size(); ++i) {
        std::memcpy(
            data.data() + currentOffset,
            values[i].data(),
            values[i].size());
        offsets[i] = currentOffset;
        currentOffset += static_cast<uint8_t>(values[i].size());
      }
      offsets[offsets.size() - 1] = currentOffset;
      std::vector<std::byte> offsetsData = GetValuesAsBytes(offsets);

      PropertyTablePropertyView<std::string_view> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(),
          gsl::span<const std::byte>(offsetsData.data(), offsetsData.size()),
          PropertyComponentType::None,
          PropertyComponentType::Uint8,
          0,
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<double> expected{
          0.0,
          static_cast<double>(10),
          static_cast<double>(-2)};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetFloat64(
                property,
                static_cast<int64>(i),
                0.0),
            expected[i]);
      }
    });
  });

  Describe("GetIntPoint", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetIntPoint(
              property,
              0,
              FIntPoint(0)),
          FIntPoint(0));
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<glm::ivec2> values{
          glm::ivec2(1, 1),
          glm::ivec2(-1, -1),
          glm::ivec2(10, 4)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::ivec2> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetIntPoint(
              property,
              -1,
              FIntPoint(0)),
          FIntPoint(0));
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetIntPoint(
              property,
              10,
              FIntPoint(0)),
          FIntPoint(0));
    });

    It("gets from glm::ivec2 property", [this]() {
      std::vector<glm::ivec2> values{
          glm::ivec2(1, 1),
          glm::ivec2(-1, -1),
          glm::ivec2(10, 4)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::ivec2> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        FIntPoint expected(values[i][0], values[i][1]);
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetIntPoint(
                property,
                static_cast<int64>(i),
                FIntPoint(0)),
            expected);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<glm::vec3> values{
          glm::vec3(1.0f, 2.0f, 3.0f),
          glm::vec3(20.5f, -1.5f, std::numeric_limits<float>::lowest()),
          glm::vec3(std::numeric_limits<float>::max(), -1.0f, 2.0f)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::vec3> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<FIntPoint> expected{
          FIntPoint(1, 2),
          FIntPoint(20, -1),
          FIntPoint(0)};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetIntPoint(
                property,
                static_cast<int64>(i),
                FIntPoint(0)),
            expected[i]);
      }
    });
  });

  Describe("GetVector2D", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector2D(
              property,
              0,
              FVector2D::Zero()),
          FVector2D::Zero());
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<glm::dvec2> values{
          glm::dvec2(-1.0, 2.0),
          glm::dvec2(3.4, 5.6),
          glm::dvec2(1.5, -1.5)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dvec2> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector2D(
              property,
              -1,
              FVector2D::Zero()),
          FVector2D::Zero());
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector2D(
              property,
              10,
              FVector2D::Zero()),
          FVector2D::Zero());
    });

    It("gets from glm::dvec2 property", [this]() {
      std::vector<glm::dvec2> values{
          glm::dvec2(-1.0, 2.0),
          glm::dvec2(3.4, 5.6),
          glm::dvec2(1.5, -1.5)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dvec2> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        FVector2D expected(values[i][0], values[i][1]);
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetVector2D(
                property,
                static_cast<int64>(i),
                FVector2D::Zero()),
            expected);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<std::string> values{"X=10 Y=3", "not a vector", "X=-2 Y=4"};
      size_t totalSize = 0;
      for (const auto& value : values) {
        totalSize += value.size();
      }

      std::vector<std::byte> data(totalSize);
      std::vector<uint8_t> offsets(values.size() + 1);
      uint8_t currentOffset = 0;
      for (size_t i = 0; i < values.size(); ++i) {
        std::memcpy(
            data.data() + currentOffset,
            values[i].data(),
            values[i].size());
        offsets[i] = currentOffset;
        currentOffset += static_cast<uint8_t>(values[i].size());
      }
      offsets[offsets.size() - 1] = currentOffset;
      std::vector<std::byte> offsetsData = GetValuesAsBytes(offsets);

      PropertyTablePropertyView<std::string_view> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(),
          gsl::span<const std::byte>(offsetsData.data(), offsetsData.size()),
          PropertyComponentType::None,
          PropertyComponentType::Uint8,
          0,
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<FVector2D> expected{
          FVector2D(10, 3),
          FVector2D::Zero(),
          FVector2D(-2, 4)};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetVector2D(
                property,
                static_cast<int64>(i),
                FVector2D::Zero()),
            expected[i]);
      }
    });
  });

  Describe("GetIntVector", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetIntVector(
              property,
              0,
              FIntVector(0)),
          FIntVector(0));
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<glm::ivec3> values{
          glm::ivec3(1, 1, -1),
          glm::ivec3(-1, -1, 2),
          glm::ivec3(10, 4, 5)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::ivec3> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetIntVector(
              property,
              -1,
              FIntVector(0)),
          FIntVector(0));
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetIntVector(
              property,
              10,
              FIntVector(0)),
          FIntVector(0));
    });

    It("gets from glm::ivec3 property", [this]() {
      std::vector<glm::ivec3> values{
          glm::ivec3(1, 1, -1),
          glm::ivec3(-1, -1, 2),
          glm::ivec3(10, 4, 5)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::ivec3> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        FIntVector expected(values[i][0], values[i][1], values[i][2]);
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetIntVector(
                property,
                static_cast<int64>(i),
                FIntVector(0)),
            expected);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<glm::vec3> values{
          glm::vec3(1.0f, 2.0f, 3.0f),
          glm::vec3(-5.9f, 8.2f, 1.15f),
          glm::vec3(20.5f, -1.5f, std::numeric_limits<float>::lowest()),
          glm::vec3(std::numeric_limits<float>::max(), -1.0f, 2.0f)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::vec3> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<FIntVector> expected{
          FIntVector(1, 2, 3),
          FIntVector(-5, 8, 1),
          FIntVector(0),
          FIntVector(0)};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetIntVector(
                property,
                static_cast<int64>(i),
                FIntVector(0)),
            expected[i]);
      }
    });
  });

  Describe("GetVector3f", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector3f(
              property,
              0,
              FVector3f::Zero()),
          FVector3f::Zero());
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<glm::vec3> values{
          glm::vec3(1.0f, 1.9f, -1.0f),
          glm::vec3(-1.0f, -1.8f, 2.5f),
          glm::vec3(10.0f, 4.4f, 5.4f)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::vec3> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector3f(
              property,
              -1,
              FVector3f::Zero()),
          FVector3f::Zero());
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector3f(
              property,
              10,
              FVector3f::Zero()),
          FVector3f::Zero());
    });

    It("gets from glm::ivec3 property", [this]() {
      std::vector<glm::vec3> values{
          glm::vec3(1.0f, 1.9f, -1.0f),
          glm::vec3(-1.0f, -1.8f, 2.5f),
          glm::vec3(10.0f, 4.4f, 5.4f)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::vec3> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        FVector3f expected(values[i][0], values[i][1], values[i][2]);
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetVector3f(
                property,
                static_cast<int64>(i),
                FVector3f(0)),
            expected);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<glm::dvec2> values{
          glm::dvec2(1.0, 2.0),
          glm::dvec2(-5.9, 8.2),
          glm::dvec2(20.5, std::numeric_limits<double>::lowest()),
          glm::dvec2(std::numeric_limits<double>::max(), -1.0)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dvec2> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<FVector3f> expected(4);
      for (size_t i = 0; i < 2; i++) {
        expected[i] = FVector3f(
            static_cast<float>(values[i][0]),
            static_cast<float>(values[i][1]),
            0.0f);
      }
      expected[2] = expected[3] = FVector3f::Zero();

      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetVector3f(
                property,
                static_cast<int64>(i),
                FVector3f::Zero()),
            expected[i]);
      }
    });
  });

  Describe("GetVector", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector(
              property,
              0,
              FVector::Zero()),
          FVector::Zero());
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<glm::dvec3> values{
          glm::dvec3(-1.0, 2.0, 5.0),
          glm::dvec3(3.4, 5.6, 7.8),
          glm::dvec3(1.5, -1.5, -2.01)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dvec3> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector(
              property,
              -1,
              FVector::Zero()),
          FVector::Zero());
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector(
              property,
              10,
              FVector::Zero()),
          FVector::Zero());
    });

    It("gets from glm::dvec3 property", [this]() {
      std::vector<glm::dvec3> values{
          glm::dvec3(-1.0, 2.0, 5.0),
          glm::dvec3(3.4, 5.6, 7.8),
          glm::dvec3(1.5, -1.5, -2.01)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dvec3> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        FVector expected(values[i][0], values[i][1], values[i][2]);
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetVector(
                property,
                static_cast<int64>(i),
                FVector::Zero()),
            expected);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<std::string> values{
          "X=10 Y=3 Z=4",
          "not a vector",
          "X=-2 Y=4 Z=5"};
      size_t totalSize = 0;
      for (const auto& value : values) {
        totalSize += value.size();
      }

      std::vector<std::byte> data(totalSize);
      std::vector<uint8_t> offsets(values.size() + 1);
      uint8_t currentOffset = 0;
      for (size_t i = 0; i < values.size(); ++i) {
        std::memcpy(
            data.data() + currentOffset,
            values[i].data(),
            values[i].size());
        offsets[i] = currentOffset;
        currentOffset += static_cast<uint8_t>(values[i].size());
      }
      offsets[offsets.size() - 1] = currentOffset;
      std::vector<std::byte> offsetsData = GetValuesAsBytes(offsets);

      PropertyTablePropertyView<std::string_view> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(),
          gsl::span<const std::byte>(offsetsData.data(), offsetsData.size()),
          PropertyComponentType::None,
          PropertyComponentType::Uint8,
          0,
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<FVector> expected{
          FVector(10, 3, 4),
          FVector::Zero(),
          FVector(-2, 4, 5)};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetVector(
                property,
                static_cast<int64>(i),
                FVector::Zero()),
            expected[i]);
      }
    });
  });

  Describe("GetVector4", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector4(
              property,
              0,
              FVector4::Zero()),
          FVector4::Zero());
    });

    It("returns default value for invalid feature ID", [this]() {
      std::vector<glm::dvec4> values{
          glm::dvec4(-1.0, 2.0, 5.0, 8.8),
          glm::dvec4(3.4, 5.6, 7.8, 0.2),
          glm::dvec4(1.5, -1.5, -2.01, 5.5)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dvec4> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector4(
              property,
              -1,
              FVector4::Zero()),
          FVector4::Zero());
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetVector4(
              property,
              10,
              FVector4::Zero()),
          FVector4::Zero());
    });

    It("gets from glm::dvec3 property", [this]() {
      std::vector<glm::dvec4> values{
          glm::dvec4(-1.0, 2.0, 5.0, 8.8),
          glm::dvec4(3.4, 5.6, 7.8, 0.2),
          glm::dvec4(1.5, -1.5, -2.01, 5.5)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dvec4> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        FVector4 expected(
            values[i][0],
            values[i][1],
            values[i][2],
            values[i][3]);
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetVector(
                property,
                static_cast<int64>(i),
                FVector4::Zero()),
            expected);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<std::string> values{
          "X=10 Y=3 Z=4 W=2",
          "not a vector",
          "X=-2 Y=4 Z=5"};
      size_t totalSize = 0;
      for (const auto& value : values) {
        totalSize += value.size();
      }

      std::vector<std::byte> data(totalSize);
      std::vector<uint8_t> offsets(values.size() + 1);
      uint8_t currentOffset = 0;
      for (size_t i = 0; i < values.size(); ++i) {
        std::memcpy(
            data.data() + currentOffset,
            values[i].data(),
            values[i].size());
        offsets[i] = currentOffset;
        currentOffset += static_cast<uint8_t>(values[i].size());
      }
      offsets[offsets.size() - 1] = currentOffset;
      std::vector<std::byte> offsetsData = GetValuesAsBytes(offsets);

      PropertyTablePropertyView<std::string_view> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(),
          gsl::span<const std::byte>(offsetsData.data(), offsetsData.size()),
          PropertyComponentType::None,
          PropertyComponentType::Uint8,
          0,
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<FVector> expected{
          FVector4(10, 3, 4, 2),
          FVector::Zero(),
          FVector4(-2, 4, 5, 1)};
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetVector4(
                property,
                static_cast<int64>(i),
                FVector4::Zero()),
            expected[i]);
      }
    });
  });

  Describe("GetMatrix", [this]() {
    It("returns default value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "value",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetMatrix(
              property,
              0,
              FMatrix::Identity),
          FMatrix::Identity);
    });

    It("returns default value for invalid feature ID", [this]() {
      // clang-format off
      std::vector<glm::dmat4> values{
          glm::dmat4(
             1.0,  2.0,  3.0,  4.0,
             5.0,  6.0,  7.0,  8.0,
             9.0, 10.0, 11.0, 12.0,
            13.0, 14.0, 15.0, 16.0),
          glm::dmat4(
             1.0,  0.0, 0.0, 0.0,
             0.0, -2.5, 0.0, 0.0,
             0.0,  0.0, 0.5, 0.0,
            -1.5,  4.0, 2.0, 1.0),
      };
      // clang-format on
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dmat4> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      TestEqual(
          "negative index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetMatrix(
              property,
              -1,
              FMatrix::Identity),

          FMatrix::Identity);
      TestEqual(
          "out-of-range positive index",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetMatrix(
              property,
              10,
              FMatrix::Identity),
          FMatrix::Identity);
    });

    It("gets from glm::dmat4 property", [this]() {
      // clang-format off
      std::vector<glm::dmat4> values{
          glm::dmat4(
             1.0,  2.0,  3.0,  4.0,
             5.0,  6.0,  7.0,  8.0,
             9.0, 10.0, 11.0, 12.0,
            13.0, 14.0, 15.0, 16.0),
          glm::dmat4(
             1.0,  0.0, 0.0, 0.0,
             0.0, -2.5, 0.0, 0.0,
             0.0,  0.0, 0.5, 0.0,
            -1.5,  4.0, 2.0, 1.0),
      };
      // clang-format on
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::dmat4> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<FMatrix> expected(2);
      expected[0] = FMatrix(
          FPlane4d(1.0, 5.0, 9.0, 13.0),
          FPlane4d(2.0, 6.0, 10.0, 14.0),
          FPlane4d(3.0, 7.0, 11.0, 15.0),
          FPlane4d(4.0, 8.0, 12.0, 16.0));
      expected[1] = FMatrix(
          FPlane4d(1.0, 0.0, 0.0, -1.5),
          FPlane4d(0.0, -2.5, 0.0, 4.0),
          FPlane4d(0.0, 0.0, 0.5, 2.0),
          FPlane4d(0.0, 0.0, 0.0, 1.0));

      for (size_t i = 0; i < values.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetMatrix(
                property,
                static_cast<int64>(i),
                FMatrix::Identity),
            expected[i]);
      }
    });

    It("converts compatible values", [this]() {
      std::vector<double> values{-2.0, 10.5};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<double> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      std::vector<FMatrix> expected(2);
      expected[0] = FMatrix(
          FPlane4d(-2.0, 0.0, 0.0, 0.0),
          FPlane4d(0.0, -2.0, 0.0, 0.0),
          FPlane4d(0.0, 0.0, -2.0, 0.0),
          FPlane4d(0.0, 0.0, 0.0, -2.0));
      expected[1] = FMatrix(
          FPlane4d(10.5, 0.0, 0.0, 0.0),
          FPlane4d(0.0, 10.5, 0.0, 0.0),
          FPlane4d(0.0, 0.0, 10.5, 0.0),
          FPlane4d(0.0, 0.0, 0.0, 10.5));
      for (size_t i = 0; i < expected.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetMatrix(
                property,
                static_cast<int64>(i),
                FMatrix::Identity),
            expected[i]);
      }
    });

    It("returns default values for incompatible type", [this]() {
      std::vector<glm::vec2> values{
          glm::vec2(-2.0f, 10.5f),
          glm::vec2(1.5f, 0.1f)};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<glm::vec2> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      for (size_t i = 0; i < values.size(); i++) {
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumPropertyTablePropertyBlueprintLibrary::GetMatrix(
                property,
                static_cast<int64>(i),
                FMatrix::Identity),
            FMatrix::Identity);
      }
    });
  });

  Describe("GetArray", [this]() {
    It("returns empty array for non-array property", [this]() {
      std::vector<int32_t> values{1, 2, 3, 4, 5, 6};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<int32_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64_t>(values.size()));

      FCesiumPropertyArray array =
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArray(property, 0);
      TestEqual(
          "array size",
          UCesiumPropertyArrayBlueprintLibrary::GetSize(array),
          0);
      FCesiumMetadataValueType valueType; // Unknown type
      TestTrue(
          "array type",
          UCesiumPropertyArrayBlueprintLibrary::GetElementValueType(array) ==
              valueType);
    });

    It("returns empty array for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);

      FCesiumPropertyArray array =
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArray(property, 0);
      TestEqual(
          "array size",
          UCesiumPropertyArrayBlueprintLibrary::GetSize(array),
          0);
      FCesiumMetadataValueType valueType; // Unknown type
      TestTrue(
          "array type",
          UCesiumPropertyArrayBlueprintLibrary::GetElementValueType(array) ==
              valueType);
    });

    It("returns empty array for invalid feature ID", [this]() {
      std::vector<int32_t> values{1, 2, 3, 4, 5, 6};
      std::vector<std::byte> data = GetValuesAsBytes(values);
      int64 arraySize = 2;
      int64 size = static_cast<int64_t>(values.size()) / arraySize;

      PropertyTablePropertyView<PropertyArrayView<int32_t>> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(),
          gsl::span<const std::byte>(),
          PropertyComponentType::None,
          PropertyComponentType::None,
          arraySize,
          size,
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          size);

      FCesiumPropertyArray array =
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArray(property, -1);
      TestEqual(
          "negative index array size",
          UCesiumPropertyArrayBlueprintLibrary::GetSize(array),
          0);
      FCesiumMetadataValueType valueType; // Unknown type
      TestTrue(
          "negative index array type",
          UCesiumPropertyArrayBlueprintLibrary::GetElementValueType(array) ==
              valueType);

      array =
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArray(property, 10);
      TestEqual(
          "out-of-range positive index array size",
          UCesiumPropertyArrayBlueprintLibrary::GetSize(array),
          0);
      TestTrue(
          "out-of-range positive index array type",
          UCesiumPropertyArrayBlueprintLibrary::GetElementValueType(array) ==
              valueType);
    });

    It("returns array for fixed-length array property", [this]() {
      std::vector<int32_t> values{1, 2, 3, 4, 5, 6};
      std::vector<std::byte> data = GetValuesAsBytes(values);
      int64 arraySize = 2;
      int64 size = static_cast<int64_t>(values.size()) / arraySize;

      PropertyTablePropertyView<PropertyArrayView<int32_t>> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(),
          gsl::span<const std::byte>(),
          PropertyComponentType::None,
          PropertyComponentType::None,
          arraySize,
          size,
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          size);
      TestEqual(
          "ArraySize",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArraySize(property),
          arraySize);

      for (int64 i = 0; i < size; i++) {
        FCesiumPropertyArray array =
            UCesiumPropertyTablePropertyBlueprintLibrary::GetArray(property, i);
        TestEqual(
            "array size",
            UCesiumPropertyArrayBlueprintLibrary::GetSize(array),
            arraySize);
        FCesiumMetadataValueType valueType(
            ECesiumMetadataType::Scalar,
            ECesiumMetadataComponentType::Int32,
            false);
        TestTrue(
            "array element type",
            UCesiumPropertyArrayBlueprintLibrary::GetElementValueType(array) ==
                valueType);

        int64 arrayOffset = i * arraySize;
        for (int64 j = 0; j < arraySize; j++) {
          std::string label(
              "array" + std::to_string(i) + " value" + std::to_string(j));
          FCesiumMetadataValue value =
              UCesiumPropertyArrayBlueprintLibrary::GetValue(array, j);
          TestEqual(
              label.c_str(),
              UCesiumMetadataValueBlueprintLibrary::GetInteger(value, 0),
              values[static_cast<size_t>(arrayOffset + j)]);
        }
      }
    });

    It("returns array for variable-length array property", [this]() {
      std::vector<int32_t> values{1, 2, 3, 4, 5, 6};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      std::vector<uint16_t> offsets{
          0,
          2 * sizeof(int32_t),
          3 * sizeof(int32_t),
          6 * sizeof(int32_t)};
      std::vector<std::byte> offsetsData = GetValuesAsBytes(offsets);
      int64 size = static_cast<int64_t>(offsets.size() - 1);

      PropertyTablePropertyView<PropertyArrayView<int32_t>> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          gsl::span<const std::byte>(offsetsData.data(), offsetsData.size()),
          gsl::span<const std::byte>(),
          PropertyComponentType::Uint16,
          PropertyComponentType::None,
          0,
          size,
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          size);
      TestEqual(
          "ArraySize",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetArraySize(property),
          0);

      std::vector<std::vector<int32_t>> expected{{1, 2}, {3}, {4, 5, 6}};
      for (int64 i = 0; i < size; i++) {
        const std::vector<int32_t>& expectedArray =
            expected[static_cast<size_t>(i)];
        FCesiumPropertyArray array =
            UCesiumPropertyTablePropertyBlueprintLibrary::GetArray(property, i);
        TestEqual(
            "array size",
            UCesiumPropertyArrayBlueprintLibrary::GetSize(array),
            static_cast<int64>(expectedArray.size()));
        FCesiumMetadataValueType valueType(
            ECesiumMetadataType::Scalar,
            ECesiumMetadataComponentType::Int32,
            false);
        TestTrue(
            "element type",
            UCesiumPropertyArrayBlueprintLibrary::GetElementValueType(array) ==
                valueType);

        for (size_t j = 0; j < expectedArray.size(); j++) {
          std::string label(
              "array" + std::to_string(i) + " value" + std::to_string(j));
          FCesiumMetadataValue value =
              UCesiumPropertyArrayBlueprintLibrary::GetValue(
                  array,
                  static_cast<int64>(j));
          TestEqual(
              label.c_str(),
              UCesiumMetadataValueBlueprintLibrary::GetInteger(value, 0),
              expectedArray[j]);
        }
      }
    });
  });

  Describe("GetValue", [this]() {
    It("returns empty value for invalid property", [this]() {
      FCesiumPropertyTableProperty property;
      TestEqual(
          "PropertyTablePropertyStatus",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::ErrorInvalidProperty);
      TestEqual(
          "Size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          0);

      FCesiumMetadataValue value =
          UCesiumPropertyTablePropertyBlueprintLibrary::GetValue(property, 0);
      FCesiumMetadataValueType valueType; // Unknown type
      TestTrue(
          "value type",
          UCesiumMetadataValueBlueprintLibrary::GetValueType(value) ==
              valueType);
    });

    It("returns empty value for invalid feature ID", [this]() {
      std::vector<int32_t> values{-1, 2, -3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<int32_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      FCesiumMetadataValue value =
          UCesiumPropertyTablePropertyBlueprintLibrary::GetValue(property, -1);
      FCesiumMetadataValueType valueType; // Unknown type
      TestTrue(
          "negative index value type",
          UCesiumMetadataValueBlueprintLibrary::GetValueType(value) ==
              valueType);

      value =
          UCesiumPropertyTablePropertyBlueprintLibrary::GetValue(property, 10);
      TestTrue(
          "out-of-range positive index value type",
          UCesiumMetadataValueBlueprintLibrary::GetValueType(value) ==
              valueType);
    });

    It("gets value for valid feature IDs", [this]() {
      std::vector<int32_t> values{-1, 2, -3, 4};
      std::vector<std::byte> data = GetValuesAsBytes(values);

      PropertyTablePropertyView<int32_t> propertyView(
          gsl::span<const std::byte>(data.data(), data.size()),
          static_cast<int64_t>(values.size()),
          false);
      FCesiumPropertyTableProperty property(propertyView);
      TestEqual(
          "status",
          UCesiumPropertyTablePropertyBlueprintLibrary::
              GetPropertyTablePropertyStatus(property),
          ECesiumPropertyTablePropertyStatus::Valid);
      TestEqual(
          "size",
          UCesiumPropertyTablePropertyBlueprintLibrary::GetPropertySize(
              property),
          static_cast<int64>(values.size()));

      FCesiumMetadataValueType valueType(
          ECesiumMetadataType::Scalar,
          ECesiumMetadataComponentType::Int32,
          false);
      for (size_t i = 0; i < values.size(); i++) {
        FCesiumMetadataValue value =
            UCesiumPropertyTablePropertyBlueprintLibrary::GetValue(property, i);
        TestTrue(
            "value type",
            UCesiumMetadataValueBlueprintLibrary::GetValueType(value) ==
                valueType);
        TestEqual(
            std::string("value" + std::to_string(i)).c_str(),
            UCesiumMetadataValueBlueprintLibrary::GetInteger(value, 0),
            values[i]);
      }
    });
  });
}
