#include "ElevationConfig.h"
#include "rapidxml.hpp"
#include <fstream>
#include <string>
#include <functional>
#include <queue>

namespace GIS {

	namespace {

		using ConfigValue = ElevationConfig::Value;

		struct ParseResult {

			struct Array : public std::vector<std::pair<ElevationConfig::Key, ElevationConfig::Value>> {

				template <typename T>
				Array & Add(const ElevationConfig::Key & key, const T & t) {
					push_back({ key, ElevationConfig::Value { t } });
					return *this;
				}

			};

		public:

			bool isParseChild = true;

			Array array;

		};

		using ParseFunction = std::function<ParseResult(rapidxml::xml_node<>*)>;

		using XMLNode = rapidxml::xml_node<>;

		// Parse data cache name
		ParseResult ParseDataCacheName(XMLNode*node);

		template <typename T>
		bool GetValue(XMLNode *node, T & value) {
			if (node) {
				std::istringstream parser{ node->value() };
				parser >> value;
				return parser.good();
			}
			else {
				return false;
			}
		}

		template <>
		bool GetValue(XMLNode*node, LatLon & value) {
			if (auto units = node->first_attribute("units")) {
				float latitude = 0;
				if (auto latAttr = node->first_attribute("latitude"))
					latitude = std::stof(latAttr->value());
				else
					return false;
				float longitude = 0;
				if (auto lonAttr = node->first_attribute("longitude"))
					longitude = std::stof(lonAttr->value());
				else
					return false;
				if (units->value() == std::string("degrees")) {
					value = value.FromDegrees(latitude, longitude);
				}
				else {
					value = value.FromRadians(latitude, longitude);
				}
				return true;
			}
			else {
				return false;
			}
		}

		// Parse Tile Size
		ParseResult ParseTileSize(XMLNode *node);

		// Parse Data Type
		ParseResult ParseDataType(XMLNode *node);

		// Parse data detail hint
		ParseResult ParseDetailHint(XMLNode *node);

		// Parse map url
		ParseResult ParseMapUrl(XMLNode*node);

		// Parse layer
		ParseResult ParseLayerNames(XMLNode*node);

		// Parse level zero tile data
		ParseResult ParseLevelZeroTileDelta(XMLNode *node);

		// Parse number levels
		ParseResult ParseNumLevels(XMLNode *node);

		// Parse sector
		ParseResult ParseSector(XMLNode *node);

		// Parse format suffix
		ParseResult ParseFormatSuffix(XMLNode *node);

		// Parse tile origin
		ParseResult ParseTileOrigin(XMLNode *node);

		// Parse image format
		ParseResult ParseImageFormat(XMLNode *node);

		// Parse elevation extreme
		ParseResult ParseElevationExtreme(XMLNode *node);

		bool GetParser(const std::string & xmlNodeName, ParseFunction & function)
		{
			using NodeName = std::string;
			static std::unordered_map<NodeName, ParseFunction> map
			{
				// Register parse function
				std::make_pair("DataCacheName", ParseDataCacheName),
				std::make_pair("TileSize", ParseTileSize),
				std::make_pair("DataType", ParseDataType),
				std::make_pair("DataDetailHint", ParseDetailHint ),
				std::make_pair("GetMapURL", ParseMapUrl ),
				std::make_pair("LayerNames", ParseLayerNames ),
				std::make_pair("LevelZeroTileDelta", ParseLevelZeroTileDelta),
				std::make_pair("NumLevels", ParseNumLevels),
				std::make_pair("Sector", ParseSector ),
				std::make_pair("FormatSuffix", ParseFormatSuffix ),
				std::make_pair("ImageFormat", ParseImageFormat ),
				std::make_pair("TileOrigin", ParseTileOrigin ),
				std::make_pair("ExtremeElevations", ParseElevationExtreme )
			};
			auto findResult = map.find(xmlNodeName);
			if (findResult != map.end())
			{
				function = findResult->second;
				return true;
			}
			else
			{
				return false;
			}
		}

		bool ParseLatLon(XMLNode *node, LatLon *latLon ) {
			if (auto latAttr = node->first_attribute("latitude")) 
			{
				float lat = 0;
				lat = std::stof( latAttr->value() );
				if (auto lonAttr = node->first_attribute("longitude"))
				{
					float lon = 0;
					lon = std::stof(lonAttr->value());
					if( auto unitsAttr = node->first_attribute( "units" ) ) 
					{
						if (std::string("degrees") == unitsAttr->value()) {
							*latLon = LatLon::FromDegrees(lat, lon);
						}
						else {
							*latLon = LatLon::FromRadians(lat, lon);
						}
						return true;
					}
				}
			}
			return false;
		}

		ParseResult ParseDataCacheName(XMLNode* node)
		{
			ParseResult result;
			result.isParseChild = false;
			result.array.Add(GIS::ConfigKey::DATA_CACHE_NAME, std::string(node->value()));
			return result;
		}

		ParseResult ParseTileSize(XMLNode * node)
		{
			ParseResult result;
			result.isParseChild = false;
			if (auto dimensionNode = node->first_node("Dimension")) {
				std::pair<int, int> dimension;
				if (auto widthAttr = dimensionNode->first_attribute("width")) {
					dimension.first = std::stoi( widthAttr->value() );
				}
				if (auto heightAttr = dimensionNode->first_attribute("height")) {
					dimension.second = std::stoi(heightAttr->value());
				}
				result.array.Add(ConfigKey::TILE_WIDTH, dimension.first);
				result.array.Add(ConfigKey::TILE_HEIGHT, dimension.second);
			}
			return result;
		}

		ParseResult ParseDataType(XMLNode * node)
		{
			ParseResult result;
			result.isParseChild = false;
			if (auto typeAttr = node->first_attribute("type"))
			{
				auto type = std::string(typeAttr->value());
				ElevationConfig::DataType dataType = ElevationConfig::DataType::Int16;
				if ("Int16" == type) {
					dataType = ElevationConfig::DataType::Int16;
				}
				else if ("Int32" == type)
				{
					dataType = ElevationConfig::DataType::Int32;
					
				}
				else if ("Float32" == type) 
				{
					dataType = ElevationConfig::DataType::Float32;
				}
				else if ("Float64" == type) {
					dataType = ElevationConfig::DataType::Float64;
				}
				else {
					throw std::runtime_error{ "Unsupported data type" };
				}

				result.array.push_back({ ConfigKey::DATA_TYPE, ConfigValue { dataType } });
			}
			if (auto byteOrderAttr = node->first_attribute("byteOrder"))
			{
				auto byteOrder = std::string(byteOrderAttr->value());
				result.array.Add( ConfigKey::BYTE_ORDER, 
					byteOrder == "LittleEndian" ? ElevationConfig::ByteOrder::LittleEndian : ElevationConfig::ByteOrder::BigEndian );
			}
			return result;
		}

		ParseResult ParseDetailHint(XMLNode *node) {
			ParseResult result;
			result.array.Add( ConfigKey::DETAIL_HINT, std::stof(node->value()));
			return result;
		}

		ParseResult ParseMapUrl(XMLNode * node)
		{
			ParseResult result;
			result.array.Add(ConfigKey::GET_MAP_URL, node->value());
			return result;
		}

		ParseResult ParseLayerNames(XMLNode * node)
		{
			ParseResult result;
			result.array.Add(ConfigKey::LAYER_NAME, node->value());
			return result;
		}

		ParseResult ParseLevelZeroTileDelta(XMLNode * node)
		{
			ParseResult result;
			LatLon latLon;
			if (GetValue(node->first_node("LatLon" ), latLon)) {
				result.array.Add(ConfigKey::LEVEL_ZERO_TILE_DELTA, latLon);
			}
			return result;
		}

		ParseResult ParseNumLevels(XMLNode * node)
		{
			ParseResult result;
			if (auto countAttr = node->first_attribute("count")) {
				auto count = std::stoi(countAttr->value());
				result.array.Add(ConfigKey::NUM_LEVELS, count);
			}
			return result;
		}

		ParseResult ParseSector(XMLNode * node)
		{
			ParseResult result;
			LatLon southWest, northEast;
			if (auto southWestNode = node->first_node("SouthWest")) 
			{
				if (auto latLonNode = southWestNode->first_node("LatLon")) 
				{
					ParseLatLon(latLonNode, &southWest);
				}
			}
			if (auto northEastNode = node->first_node("NorthEast"))
			{
				if (auto latLonNode = northEastNode->first_node("LatLon")) 
				{
					ParseLatLon(latLonNode, &northEast);
				}
			}
			result.array.Add(ConfigKey::SECTOR, Sector{
					southWest.GetLatitude(),
					northEast.GetLatitude(),
					southWest.GetLongitude(),
					northEast.GetLongitude() });
			return result;
		}

		ParseResult ParseFormatSuffix(XMLNode *node)
		{
			ParseResult result;
			if (auto suffix = node->value())
				result.array.Add(ConfigKey::FORMAT_SUFFIX, suffix);
			return result;
		}

		ParseResult ParseTileOrigin(XMLNode * node)
		{
			ParseResult result;
			LatLon latLon;
			if (GetValue(node->first_node("LatLon"), latLon))
			{
				result.array.Add(ConfigKey::TILE_ORIGIN, latLon);
			}
			return result;
		}

		ParseResult ParseImageFormat(XMLNode * node)
		{
			ParseResult result;
			if (auto imageFormat = node->value())
				result.array.Add(ConfigKey::IMAGE_FORMAT, imageFormat);
			return result;
		}

		ParseResult ParseElevationExtreme(XMLNode *node)
		{
			ParseResult result;
			if (auto minAttr = node->first_attribute("min"))
			{
				result.array.push_back({ ConfigKey::ELEVATION_MIN, ConfigValue { std::stof(minAttr->value()) } });
			}
			if (auto maxAttr = node->first_attribute("max"))
			{
				result.array.push_back({ ConfigKey::ELEVATION_MAX, ConfigValue { std::stof(maxAttr->value()) } });
			}
			if (auto fileNameNode = node->first_node("FileName"))
			{
				result.array.push_back({ ConfigKey::ELEVATION_EXTREMES_FILE, ConfigValue { std::string{ fileNameNode->value() } } } );
			}
			return result;
		} 

	}

}

GIS::ElevationConfig::ElevationConfig(const std::string & xmlFiles)
{
	SetValue(ConfigKey::MODEL_STORE_PATH, R"(C:\model\Vking3DGISData)" );
	std::ifstream input{ xmlFiles };
	if (input.is_open()) 
	{
		input.seekg(0, std::ios::end );
		auto size  = static_cast<size_t>( input.tellg() );
		input.seekg(0, std::ios::beg);
		std::string content;
		content.resize(size);
		input.read(&content[0], size);
		
		rapidxml::xml_document<> doc;
		doc.parse<0>(const_cast<char*>(content.c_str()));

		std::queue<rapidxml::xml_node<>*> pendingNodes;
		
		pendingNodes.push( doc.first_node() );
		while (!pendingNodes.empty()) 
		{
			auto currentNode = pendingNodes.front();
			pendingNodes.pop();
			auto nodeName = currentNode->name();
			ParseFunction function;
			if (GetParser(nodeName, function)) {
				auto result = function(currentNode);
				for( auto & elem : result.array )
				map_[elem.first] = elem.second;
				if (!result.isParseChild)
					continue;
			}
			auto child = currentNode->first_node();
			while (child)
			{
				pendingNodes.push(child);
				child = child->next_sibling();
			}
		}
	}

}
