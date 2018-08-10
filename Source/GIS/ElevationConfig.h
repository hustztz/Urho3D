#pragma once

#include "Variant.h"
#include "LatLon.h"
#include "Sector.h"
#include <string>
#include <unordered_map>
#include <sstream>
#include <variant>

namespace GIS
{
	namespace ConfigKey {
		\
		const std::string NORTHWEST = "gov.nasa.worldwind.layers.ViewControlsLayer.NorthWest";
		const std::string SOUTHWEST = "gov.nasa.worldwind.layers.ViewControlsLayer.SouthWest";
		const std::string NORTHEAST = "gov.nasa.worldwind.layers.ViewControlsLayer.NorthEast";
		const std::string SOUTHEAST = "gov.nasa.worldwind.layers.ViewControlsLayer.SouthEast";

		// Start alphabetic order
		const std::string ABOVE_GROUND_LEVEL = "gov.nasa.worldwind.avkey.AboveGroundLevel";
		const std::string ABOVE_GROUND_REFERENCE = "gov.nasa.worldwind.avkey.AboveGroundReference";
		const std::string ABOVE_MEAN_SEA_LEVEL = "gov.nasa.worldwind.avkey.AboveMeanSeaLevel";
		const std::string ACTION = "gov.nasa.worldwind.avkey.Action";
		const std::string AIRSPACE_GEOMETRY_CACHE_SIZE = "gov.nasa.worldwind.avkey.AirspaceGeometryCacheSize";
		const std::string ALLOW = "gov.nasa.worldwind.avkey.Allow";
		const std::string AUTH_TOKEN = "gov.nasa.worldwind.avkey.AuthToken";

		const std::string AVAILABLE_IMAGE_FORMATS = "gov.nasa.worldwind.avkey.AvailableImageFormats";
		const std::string AVERAGE_TILE_SIZE = "gov.nasa.worldwind.avkey.AverageTileSize";

		const std::string BALLOON = "gov.nasa.worldwind.avkey.Balloon";
		const std::string BALLOON_TEXT = "gov.nasa.worldwind.avkey.BalloonText";
		const std::string BACK = "gov.nasa.worldwind.avkey.Back";
		const std::string BEGIN = "gov.nasa.worldwind.avkey.Begin";
		const std::string BIG_ENDIAN = "gov.nasa.worldwind.avkey.BigEndian";
		const std::string BOTTOM = "gov.nasa.worldwind.avkey.Bottom";
		const std::string BYTE_ORDER = "gov.nasa.worldwind.avkey.ByteOrder";
		const std::string BANDS_ORDER = "gov.nasa.worldwind.avkey.BandsOrder";

		const std::string BLACK_GAPS_DETECTION = "gov.nasa.worldwind.avkey.DetectBlackGaps";
		const std::string BOUNDS = "gov.nasa.worldwind.avkey.Bounds";

		const std::string CACHE_CONTENT_TYPES = "gov.nasa.worldwind.avkey.CacheContentTypes";
		const std::string CENTER = "gov.nasa.worldwind.avkey.Center";

		const std::string CLASS_LEVEL = "gov.nasa.worldwind.avkey.ClassLevel";
		const std::string CLASS_LEVEL_UNCLASSIFIED = "gov.nasa.worldwind.avkey.ClassLevel.Unclassified";
		const std::string CLASS_LEVEL_RESTRICTED = "gov.nasa.worldwind.avkey.ClassLevel.Restricted";
		const std::string CLASS_LEVEL_CONFIDENTIAL = "gov.nasa.worldwind.avkey.ClassLevel.Confidential";
		const std::string CLASS_LEVEL_SECRET = "gov.nasa.worldwind.avkey.ClassLevel.Secret";
		const std::string CLASS_LEVEL_TOPSECRET = "gov.nasa.worldwind.avkey.ClassLevel.TopSecret";

		const std::string CLOCKWISE = "gov.nasa.worldwind.avkey.ClockWise";
		const std::string CLOSE = "gov.nasa.worldwind.avkey.Close";
		const std::string COLOR = "gov.nasa.worldwind.avkey.Color";
		const std::string COMPRESS_TEXTURES = "gov.nasa.worldwind.avkey.CompressTextures";
		const std::string CONSTRUCTION_PARAMETERS = "gov.nasa.worldwind.avkey.ConstructionParameters";
		const std::string CONTEXT = "gov.nasa.worldwind.avkey.Context";
		const std::string COORDINATE_SYSTEM = "gov.nasa.worldwind.avkey.CoordinateSystem";
		const std::string COORDINATE_SYSTEM_GEOGRAPHIC = "gov.nasa.worldwind.avkey.CoordinateSystem.Geographic";
		const std::string COORDINATE_SYSTEM_NAME = "gov.nasa.worldwind.avkey.CoordinateSystem.Name";
		const std::string COORDINATE_SYSTEM_PROJECTED = "gov.nasa.worldwind.avkey.CoordinateSystem.Projected";
		const std::string COORDINATE_SYSTEM_SCREEN = "gov.nasa.worldwind.avkey.CoordinateSystem.Screen";
		const std::string COORDINATE_SYSTEM_UNKNOWN = "gov.nasa.worldwind.avkey.CoordinateSystem.Unknown";

		const std::string COUNTER_CLOCKWISE = "gov.nasa.worldwind.avkey.CounterClockWise";

		const std::string DATA_CACHE_NAME = "gov.nasa.worldwind.avkey.DataCacheNameKey";
		const std::string DATA_FILE_STORE_CLASS_NAME = "gov.nasa.worldwind.avkey.DataFileStoreClassName";
		const std::string DATA_FILE_STORE_CONFIGURATION_FILE_NAME
			= "gov.nasa.worldwind.avkey.DataFileStoreConfigurationFileName";
		const std::string DATASET_NAME = "gov.nasa.worldwind.avkey.DatasetNameKey";
		const std::string DATA_RASTER_READER_FACTORY_CLASS_NAME = "gov.nasa.worldwind.avkey.DataRasterReaderFactoryClassName";
		const std::string DATASET_TYPE = "gov.nasa.worldwind.avkey.DatasetTypeKey";
		const std::string DATE_TIME = "gov.nasa.worldwind.avkey.DateTime";
		/**
		 * Indicates the primitive data type of a dataset or a buffer of data. When used as a key, the corresponding value
		 * may be one of the following: <code>INT8</code>, <code>INT16</code>, <code>INT32</code>, <code>INT64</code>,
		 * <code>FLOAT32</code>, or <code>FLOAT64</code>.
		 */
		const std::string DATA_TYPE = "gov.nasa.worldwind.avkey.DataType";
		const std::string DELETE_CACHE_ON_EXIT = "gov.nasa.worldwind.avkey.DeleteCacheOnExit";
		/**
		 * Indicates the World Wind scene's worst-case depth resolution, in meters. This is typically interpreted by the
		 * View as the desired resolution at the scene's maximum drawing distance. In this case, the resolution closer to
		 * the viewer's eye point is significantly better then the worst-case resolution. Decreasing this value enables the
		 * viewer to get closer to 3D shapes positioned above the terrain at the coast of potential rendering artifacts
		 * between shapes that are places closely together or close to the terrain.
		 */
		const std::string DEPTH_RESOLUTION = "gov.nasa.worldwind.avkey.DepthResolution";
		const std::string DESCRIPTION = "gov.nasa.worldwind.avkey.Description";
		const std::string DETAIL_HINT = "gov.nasa.worldwind.avkey.DetailHint";
		const std::string DISPLAY_ICON = "gov.nasa.worldwind.avkey.DisplayIcon";
		const std::string DISPLAY_NAME = "gov.nasa.worldwind.avkey.DisplayName";

		const std::string DTED_LEVEL = "gov.nasa.worldwind.avkey.DTED.Level";

		const std::string EARTH_ELEVATION_MODEL_CAPABILITIES = "gov.nasa.worldwind.avkey.EarthElevationModelCapabilities";
		const std::string EARTH_ELEVATION_MODEL_CLASS_NAME = "gov.nasa.worldwind.avkey.EarthElevationModelClassName";
		const std::string EARTH_ELEVATION_MODEL_CONFIG_FILE = "gov.nasa.worldwind.avkey.EarthElevationModelConfigFile";
		const std::string EAST = "gov.nasa.worldwind.avkey.East";

		const std::string ELEVATION = "gov.nasa.worldwind.avkey.Elevation";
		const std::string ELEVATION_EXTREMES_FILE = "gov.nasa.worldwind.avkey.ElevationExtremesFileKey";
		const std::string ELEVATION_EXTREMES_LOOKUP_CACHE_SIZE = "gov.nasa.worldwind.avkey.ElevationExtremesLookupCacheSize";
		const std::string ELEVATION_MIN = "gov.nasa.worldwind.avkey.ElevationMinKey";
		const std::string ELEVATION_MAX = "gov.nasa.worldwind.avkey.ElevationMaxKey";
		const std::string ELEVATION_MODEL = "gov.nasa.worldwind.avkey.ElevationModel";
		const std::string ELEVATION_MODEL_FACTORY = "gov.nasa.worldwind.avkey.ElevationModelFactory";
		const std::string ELEVATION_TILE_CACHE_SIZE = "gov.nasa.worldwind.avkey.ElevationTileCacheSize";
		const std::string ELEVATION_UNIT = "gov.nasa.worldwind.avkey.ElevationUnit";

		const std::string END = "gov.nasa.worldwind.avkey.End";

		const std::string EXPIRY_TIME = "gov.nasa.worldwind.avkey.ExpiryTime";
		const std::string EXTENT = "gov.nasa.worldwind.avkey.Extent";
		const std::string EXTERNAL_LINK = "gov.nasa.worldwind.avkey.ExternalLink";

		const std::string FEEDBACK_ENABLED = "gov.nasa.worldwind.avkey.FeedbackEnabled";
		const std::string FEEDBACK_REFERENCE_POINT = "gov.nasa.worldwind.avkey.FeedbackReferencePoint";
		const std::string FEEDBACK_SCREEN_BOUNDS = "gov.nasa.worldwind.avkey.FeedbackScreenBounds";
		const std::string FILE = "gov.nasa.worldwind.avkey.File";
		const std::string FILE_NAME = "gov.nasa.worldwind.avkey.FileName";
		const std::string FILE_SIZE = "gov.nasa.worldwind.avkey.FileSize";
		const std::string FILE_STORE = "gov.nasa.worldwind.avkey.FileStore";
		const std::string FILE_STORE_LOCATION = "gov.nasa.worldwind.avkey.FileStoreLocation";
		const std::string FLOAT32 = "gov.nasa.worldwind.avkey.Float32";
		const std::string FLOAT64 = "gov.nasa.worldwind.avkey.Float64";
		const std::string FORMAT_SUFFIX = "gov.nasa.worldwind.avkey.FormatSuffixKey";
		const std::string FORWARD = "gov.nasa.worldwind.avkey.Forward";
		const std::string FOV = "gov.nasa.worldwind.avkey.FieldOfView";
		const std::string FORCE_LEVEL_ZERO_LOADS = "gov.nasa.worldwind.avkey.ForceLevelZeroLoads";
		const std::string FRACTION = "gov.nasa.worldwind.avkey.Fraction";
		const std::string FRAME_TIMESTAMP = "gov.nasa.worldwind.avkey.FrameTimestamp";

		const std::string GDAL_AREA = "gov.nasa.worldwind.avkey.GDAL.Area";
		const std::string GDAL_CACHEMAX = "gov.nasa.worldwind.avkey.GDAL.CacheMax";
		const std::string GDAL_DEBUG = "gov.nasa.worldwind.avkey.GDAL.Debug";
		const std::string GDAL_MASK_DATASET = "gov.nasa.worldwind.avkey.GDAL.MaskDataset";
		const std::string GDAL_TIMEOUT = "gov.nasa.worldwind.avkey.GDAL.TimeOut";
		const std::string GDAL_PATH = "gov.nasa.worldwind.avkey.GDAL.Path";

		const std::string GET_CAPABILITIES_URL = "gov.nasa.worldwind.avkey.GetCapabilitiesURL";
		const std::string GET_MAP_URL = "gov.nasa.worldwind.avkey.GetMapURL";
		const std::string GLOBE = "gov.nasa.worldwind.avkey.GlobeObject";
		const std::string GLOBE_CLASS_NAME = "gov.nasa.worldwind.avkey.GlobeClassName";
		const std::string GRAYSCALE = "gov.nasa.worldwind.avkey.Grayscale";
		const std::string GREAT_CIRCLE = "gov.nasa.worldwind.avkey.GreatCircle";

		const std::string HEIGHT = "gov.nasa.worldwind.avkey.Height";
		const std::string HIDDEN = "gov.nasa.worldwind.avkey.Hidden";
		const std::string HORIZONTAL = "gov.nasa.worldwind.avkey.Horizontal";
		const std::string HOT_SPOT = "gov.nasa.worldwind.avkey.HotSpot";
		const std::string HOVER_TEXT = "gov.nasa.worldwind.avkey.HoverText";
		const std::string HTTP_SSL_CONTEXT = "gov.nasa.worldwind.avkey.HTTP.SSLContext";

		const std::string ICON_NAME = "gov.nasa.worldwind.avkey.IconName";
		//const std::string IGNORE = "gov.nasa.worldwind.avkey.Ignore";
		const std::string IMAGE = "gov.nasa.worldwind.avkey.Image";
		const std::string IMAGE_FORMAT = "gov.nasa.worldwind.avkey.ImageFormat";
		/**
		 * Indicates whether an image represents color or grayscale values. When used as a key, the corresponding value may
		 * be one of the following: <code>COLOR</code> or <code>GRAYSCALE</code>.
		 */
		const std::string IMAGE_COLOR_FORMAT = "gov.nasa.worldwind.avkey.ImageColorFormat";
		const std::string INACTIVE_LEVELS = "gov.nasa.worldwind.avkey.InactiveLevels";
		const std::string INSTALLED = "gov.nasa.worldwind.avkey.Installed";
		const std::string INITIAL_ALTITUDE = "gov.nasa.worldwind.avkey.InitialAltitude";
		const std::string INITIAL_HEADING = "gov.nasa.worldwind.avkey.InitialHeading";
		const std::string INITIAL_LATITUDE = "gov.nasa.worldwind.avkey.InitialLatitude";
		const std::string INITIAL_LONGITUDE = "gov.nasa.worldwind.avkey.InitialLongitude";
		const std::string INITIAL_PITCH = "gov.nasa.worldwind.avkey.InitialPitch";
		const std::string INPUT_HANDLER_CLASS_NAME = "gov.nasa.worldwind.avkey.InputHandlerClassName";
		const std::string INSET_PIXELS = "gov.nasa.worldwind.avkey.InsetPixels";
		const std::string INT8 = "gov.nasa.worldwind.avkey.Int8";
		const std::string INT16 = "gov.nasa.worldwind.avkey.Int16";
		const std::string INT32 = "gov.nasa.worldwind.avkey.Int32";
		const std::string INT64 = "gov.nasa.worldwind.avkey.Int64";

		const std::string LAST_UPDATE = "gov.nasa.worldwind.avkey.LastUpdateKey";
		const std::string LAYER = "gov.nasa.worldwind.avkey.LayerObject";
		const std::string LAYER_ABSTRACT = "gov.nasa.worldwind.avkey.LayerAbstract";
		const std::string LAYER_DESCRIPTOR_FILE = "gov.nasa.worldwind.avkey.LayerDescriptorFile";
		const std::string LAYER_FACTORY = "gov.nasa.worldwind.avkey.LayerFactory";
		const std::string LAYER_NAME = "gov.nasa.worldwind.avkey.LayerName";
		const std::string LAYER_NAMES = "gov.nasa.worldwind.avkey.LayerNames";
		const std::string LAYERS = "gov.nasa.worldwind.avkey.LayersObject";
		const std::string LAYERS_CLASS_NAMES = "gov.nasa.worldwind.avkey.LayerClassNames";
		const std::string LEFT = "gov.nasa.worldwind.avkey.Left";
		const std::string LEFT_OF_CENTER = "gov.nasa.worldwind.avkey.LeftOfCenter";
		const std::string LEVEL_NAME = "gov.nasa.worldwind.avkey.LevelNameKey";
		const std::string LEVEL_NUMBER = "gov.nasa.worldwind.avkey.LevelNumberKey";
		const std::string LEVEL_ZERO_TILE_DELTA = "gov.nasa.worldwind.avkey.LevelZeroTileDelta";
		const std::string LINEAR = "gov.nasa.worldwind.avkey.Linear";
		const std::string LITTLE_ENDIAN = "gov.nasa.worldwind.avkey.LittleEndian";
		const std::string LOGGER_NAME = "gov.nasa.worldwind.avkey.LoggerName";
		const std::string LOXODROME = "gov.nasa.worldwind.avkey.Loxodrome";

		const std::string MAP_SCALE = "gov.nasa.worldwind.avkey.MapScale";
		const std::string MARS_ELEVATION_MODEL_CLASS_NAME = "gov.nasa.worldwind.avkey.MarsElevationModelClassName";
		const std::string MARS_ELEVATION_MODEL_CONFIG_FILE = "gov.nasa.worldwind.avkey.MarsElevationModelConfigFile";

		/**
		 * Describes the maximum number of attempts to make when downloading a resource before attempts are suspended.
		 * Attempts are restarted after the interval specified by {@link #MIN_ABSENT_TILE_CHECK_INTERVAL}.
		 *
		 * @see #MIN_ABSENT_TILE_CHECK_INTERVAL
		 */
		const std::string MAX_ABSENT_TILE_ATTEMPTS = "gov.nasa.worldwind.avkey.MaxAbsentTileAttempts";

		const std::string MAX_ACTIVE_ALTITUDE = "gov.nasa.worldwind.avkey.MaxActiveAltitude";
		const std::string MAX_MESSAGE_REPEAT = "gov.nasa.worldwind.avkey.MaxMessageRepeat";
		const std::string MEMORY_CACHE_SET_CLASS_NAME = "gov.nasa.worldwind.avkey.MemoryCacheSetClassName";
		/**
		 * Indicates the location that MIL-STD-2525 tactical symbols and tactical point graphics retrieve their icons from.
		 * When used as a key, the corresponding value must be a string indicating a URL to a remote server, a URL to a
		 * ZIP/JAR file, or a path to folder on the local file system.
		 */
		const std::string MIL_STD_2525_ICON_RETRIEVER_PATH = "gov.nasa.worldwind.avkey.MilStd2525IconRetrieverPath";
		const std::string MIME_TYPE = "gov.nasa.worldwind.avkey.MimeType";

		/**
		 * Describes the interval to wait before allowing further attempts to download a resource after the number of
		 * attempts specified by {@link #MAX_ABSENT_TILE_ATTEMPTS} are made.
		 *
		 * @see #MAX_ABSENT_TILE_ATTEMPTS
		 */
		const std::string MIN_ABSENT_TILE_CHECK_INTERVAL = "gov.nasa.worldwind.avkey.MinAbsentTileCheckInterval";
		const std::string MIN_ACTIVE_ALTITUDE = "gov.nasa.worldwind.avkey.MinActiveAltitude";

		// Implementation note: the keys MISSING_DATA_SIGNAL and MISSING_DATA_REPLACEMENT are intentionally different than
		// their actual string values. Legacy code is expecting the string values "MissingDataFlag" and "MissingDataValue",
		// respectively.
		const std::string MISSING_DATA_SIGNAL = "gov.nasa.worldwind.avkey.MissingDataFlag";
		const std::string MISSING_DATA_REPLACEMENT = "gov.nasa.worldwind.avkey.MissingDataValue";

		const std::string MODEL = "gov.nasa.worldwind.avkey.ModelObject";
		const std::string MODEL_CLASS_NAME = "gov.nasa.worldwind.avkey.ModelClassName";
		const std::string MOON_ELEVATION_MODEL_CLASS_NAME = "gov.nasa.worldwind.avkey.MoonElevationModelClassName";
		const std::string MOON_ELEVATION_MODEL_CONFIG_FILE = "gov.nasa.worldwind.avkey.MoonElevationModelConfigFile";

		const std::string NAME = "gov.nasa.worldwind.avkey.Name";
		const std::string NETWORK_STATUS_CLASS_NAME = "gov.nasa.worldwind.avkey.NetworkStatusClassName";
		const std::string NETWORK_STATUS_TEST_SITES = "gov.nasa.worldwind.avkey.NetworkStatusTestSites";
		const std::string NEXT = "gov.nasa.worldwind.avkey.Next";
		const std::string NUM_BANDS = "gov.nasa.worldwind.avkey.NumBands";
		const std::string NUM_EMPTY_LEVELS = "gov.nasa.worldwind.avkey.NumEmptyLevels";
		const std::string NUM_LEVELS = "gov.nasa.worldwind.avkey.NumLevels";
		const std::string NETWORK_RETRIEVAL_ENABLED = "gov.nasa.worldwind.avkey.NetworkRetrievalEnabled";
		const std::string NORTH = "gov.nasa.worldwind.avkey.North";

		const std::string OFFLINE_MODE = "gov.nasa.worldwind.avkey.OfflineMode";
		const std::string OPACITY = "gov.nasa.worldwind.avkey.Opacity";
		/**
		 * Indicates an object's position in a series. When used as a key, the corresponding value must be an {@link
		 * Integer} object indicating the ordinal.
		 */
		const std::string ORDINAL = "gov.nasa.worldwind.avkey.Ordinal";
		/**
		 * Indicates a list of one or more object's positions in a series. When used as a key, the corresponding value must
		 * be a {@link java.util.List} of {@link Integer} objects indicating the ordinals.
		 */
		const std::string ORDINAL_LIST = "gov.nasa.worldwind.avkey.OrdinalList";
		const std::string ORIGIN = "gov.nasa.worldwind.avkey.Origin";

		const std::string PARENT_LAYER_NAME = "gov.nasa.worldwind.avkey.ParentLayerName";

		const std::string PAUSE = "gov.nasa.worldwind.avkey.Pause";
		const std::string PICKED_OBJECT = "gov.nasa.worldwind.avkey.PickedObject";
		const std::string PICKED_OBJECT_ID = "gov.nasa.worldwind.avkey.PickedObject.ID";
		const std::string PICKED_OBJECT_PARENT_LAYER = "gov.nasa.worldwind.avkey.PickedObject.ParentLayer";
		const std::string PICKED_OBJECT_PARENT_LAYER_NAME = "gov.nasa.worldwind.avkey.PickedObject.ParentLayer.Name";
		const std::string PICKED_OBJECT_SIZE = "gov.nasa.worldwind.avkey.PickedObject.Size";
		const std::string PIXELS = "gov.nasa.worldwind.avkey.Pixels";
		/**
		 * Indicates whether a raster's pixel values represent imagery or elevation data. When used as a key, the
		 * corresponding value may be one of the following: <code>IMAGERY</code> or <code>ELEVATION</code>.
		 */
		const std::string PIXEL_FORMAT = "gov.nasa.worldwind.avkey.PixelFormat";
		const std::string PIXEL_HEIGHT = "gov.nasa.worldwind.avkey.PixelHeight";
		const std::string PIXEL_WIDTH = "gov.nasa.worldwind.avkey.PixelWidth";
		/** @deprecated Use <code>{@link #DATA_TYPE} instead.</code>. */
		const std::string PIXEL_TYPE = DATA_TYPE;

		const std::string PLACENAME_LAYER_CACHE_SIZE = "gov.nasa.worldwind.avkey.PlacenameLayerCacheSize";
		const std::string PLAY = "gov.nasa.worldwind.avkey.Play";
		const std::string POSITION = "gov.nasa.worldwind.avkey.Position";
		const std::string PREVIOUS = "gov.nasa.worldwind.avkey.Previous";

		const std::string PRODUCER_ENABLE_FULL_PYRAMID = "gov.nasa.worldwind.avkey.Producer.EnableFullPyramid";

		const std::string PROGRESS = "gov.nasa.worldwind.avkey.Progress";
		const std::string PROGRESS_MESSAGE = "gov.nasa.worldwind.avkey.ProgressMessage";

		const std::string PROJECTION_DATUM = "gov.nasa.worldwind.avkey.Projection.Datum";
		const std::string PROJECTION_DESC = "gov.nasa.worldwind.avkey.Projection.Description";
		const std::string PROJECTION_EPSG_CODE = "gov.nasa.worldwind.avkey.Projection.EPSG.Code";
		const std::string PROJECTION_HEMISPHERE = "gov.nasa.worldwind.avkey.Projection.Hemisphere";
		const std::string PROJECTION_NAME = "gov.nasa.worldwind.avkey.Projection.Name";
		const std::string PROJECTION_UNITS = "gov.nasa.worldwind.avkey.Projection.Units";
		const std::string PROJECTION_UNKNOWN = "gov.nasa.worldwind.Projection.Unknown";
		const std::string PROJECTION_UTM = "gov.nasa.worldwind.avkey.Projection.UTM";
		const std::string PROJECTION_ZONE = "gov.nasa.worldwind.avkey.Projection.Zone";

		const std::string PROPERTIES = "gov.nasa.worldwind.avkey.Properties";

		const std::string PROTOCOL = "gov.nasa.worldwind.avkey.Protocol";
		const std::string PROTOCOL_HTTP = "gov.nasa.worldwind.avkey.Protocol.HTTP";
		const std::string PROTOCOL_HTTPS = "gov.nasa.worldwind.avkey.Protocol.HTTPS";

		const std::string RECTANGLES = "gov.nasa.worldwind.avkey.Rectangles";
		const std::string REDRAW_ON_MOUSE_PRESSED = "gov.nasa.worldwind.avkey.ForceRedrawOnMousePressed";

		const std::string RELATIVE_TO_GLOBE = "gov.nasa.worldwind.avkey.RelativeToGlobe";
		const std::string RELATIVE_TO_SCREEN = "gov.nasa.worldwind.avkey.RelativeToScreen";

		const std::string RASTER_BAND_ACTUAL_BITS_PER_PIXEL = "gov.nasa.worldwind.avkey.RasterBand.ActualBitsPerPixel";
		const std::string RASTER_BAND_MIN_PIXEL_VALUE = "gov.nasa.worldwind.avkey.RasterBand.MinPixelValue";
		const std::string RASTER_BAND_MAX_PIXEL_VALUE = "gov.nasa.worldwind.avkey.RasterBand.MaxPixelValue";

		const std::string RASTER_HAS_ALPHA = "gov.nasa.worldwind.avkey.RasterHasAlpha";
		const std::string RASTER_HAS_OVERVIEWS = "gov.nasa.worldwind.avkey.Raster.HasOverviews";
		const std::string RASTER_HAS_VOIDS = "gov.nasa.worldwind.avkey.Raster.HasVoids";
		const std::string RASTER_LAYER_CLASS_NAME = "gov.nasa.worldwind.avkey.RasterLayer.ClassName";
		const std::string RASTER_PIXEL = "gov.nasa.worldwind.avkey.RasterPixel";
		const std::string RASTER_PIXEL_IS_AREA = "gov.nasa.worldwind.avkey.RasterPixelIsArea";
		const std::string RASTER_PIXEL_IS_POINT = "gov.nasa.worldwind.avkey.RasterPixelIsPoint";
		const std::string RECTANGULAR_TESSELLATOR_MAX_LEVEL = "gov.nasa.worldwind.avkey.RectangularTessellatorMaxLevel";
		const std::string REPAINT = "gov.nasa.worldwind.avkey.Repaint";
		const std::string REPEAT_NONE = "gov.nasa.worldwind.avkey.RepeatNone";
		const std::string REPEAT_X = "gov.nasa.worldwind.avkey.RepeatX";
		const std::string REPEAT_Y = "gov.nasa.worldwind.avkey.RepeatY";
		const std::string REPEAT_XY = "gov.nasa.worldwind.avkey.RepeatXY";

		const std::string RESIZE = "gov.nasa.worldwind.avkey.Resize";
		/** On window resize, scales the item to occupy a constant relative size of the viewport. */
		const std::string RESIZE_STRETCH = "gov.nasa.worldwind.CompassLayer.ResizeStretch";
		/**
		 * On window resize, scales the item to occupy a constant relative size of the viewport, but not larger than the
		 * item's inherent size scaled by the layer's item scale factor.
		 */
		const std::string RESIZE_SHRINK_ONLY = "gov.nasa.worldwind.CompassLayer.ResizeShrinkOnly";
		/** Does not modify the item size when the window changes size. */
		const std::string RESIZE_KEEP_FIXED_SIZE = "gov.nasa.worldwind.CompassLayer.ResizeKeepFixedSize";
		const std::string RETAIN_LEVEL_ZERO_TILES = "gov.nasa.worldwind.avkey.RetainLevelZeroTiles";
		const std::string RETRIEVAL_POOL_SIZE = "gov.nasa.worldwind.avkey.RetrievalPoolSize";
		const std::string RETRIEVE_PROPERTIES_FROM_SERVICE = "gov.nasa.worldwind.avkey.RetrievePropertiesFromService";
		const std::string RETRIEVAL_QUEUE_SIZE = "gov.nasa.worldwind.avkey.RetrievalQueueSize";
		const std::string RETRIEVAL_QUEUE_STALE_REQUEST_LIMIT = "gov.nasa.worldwind.avkey.RetrievalStaleRequestLimit";
		const std::string RETRIEVAL_SERVICE_CLASS_NAME = "gov.nasa.worldwind.avkey.RetrievalServiceClassName";
		const std::string RETRIEVER_FACTORY_LOCAL = "gov.nasa.worldwind.avkey.RetrieverFactoryLocal";
		const std::string RETRIEVER_FACTORY_REMOTE = "gov.nasa.worldwind.avkey.RetrieverFactoryRemote";
		const std::string RETRIEVER_STATE = "gov.nasa.worldwind.avkey.RetrieverState";
		const std::string RETRIEVAL_STATE_ERROR = "gov.nasa.worldwind.avkey.RetrievalStateError";
		const std::string RETRIEVAL_STATE_SUCCESSFUL = "gov.nasa.worldwind.avkey.RetrievalStateSuccessful";
		const std::string RHUMB_LINE = "gov.nasa.worldwind.avkey.RhumbLine";
		const std::string RIGHT = "gov.nasa.worldwind.avkey.Right";
		const std::string RIGHT_OF_CENTER = "gov.nasa.worldwind.avkey.RightOfCenter";
		const std::string ROLLOVER_TEXT = "gov.nasa.worldwind.avkey.RolloverText";

		const std::string SCHEDULED_TASK_POOL_SIZE = "gov.nasa.worldwind.avkey.ScheduledTaskPoolSize";
		const std::string SCHEDULED_TASK_SERVICE_CLASS_NAME = "gov.nasa.worldwind.avkey.ScheduledTaskServiceClassName";
		const std::string SCENE_CONTROLLER = "gov.nasa.worldwind.avkey.SceneControllerObject";
		const std::string SCENE_CONTROLLER_CLASS_NAME = "gov.nasa.worldwind.avkey.SceneControllerClassName";
		const std::string SCREEN = "gov.nasa.worldwind.avkey.ScreenObject";
		const std::string SCREEN_CREDIT = "gov.nasa.worldwind.avkey.ScreenCredit";
		const std::string SCREEN_CREDIT_LINK = "gov.nasa.worldwind.avkey.ScreenCreditLink";
		const std::string SECTOR = "gov.nasa.worldwind.avKey.Sector";
		const std::string SECTOR_BOTTOM_LEFT = "gov.nasa.worldwind.avkey.Sector.BottomLeft";
		const std::string SECTOR_BOTTOM_RIGHT = "gov.nasa.worldwind.avkey.Sector.BottomRight";
		const std::string SECTOR_GEOMETRY_CACHE_SIZE = "gov.nasa.worldwind.avkey.SectorGeometryCacheSize";
		const std::string SECTOR_RESOLUTION_LIMITS = "gov.nasa.worldwind.avkey.SectorResolutionLimits";
		const std::string SECTOR_RESOLUTION_LIMIT = "gov.nasa.worldwind.avkey.SectorResolutionLimit";
		const std::string SECTOR_UPPER_LEFT = "gov.nasa.worldwind.avkey.Sector.UpperLeft";
		const std::string SECTOR_UPPER_RIGHT = "gov.nasa.worldwind.avkey.Sector.UpperRight";
		const std::string SENDER = "gov.nasa.worldwind.avkey.Sender";
		const std::string SERVER = "gov.nasa.worldwind.avkey.Server";
		const std::string SERVICE = "gov.nasa.worldwind.avkey.ServiceURLKey";
		const std::string SERVICE_CLASS = "gov.nasa.worldwind.avkey.ServiceClass";
		const std::string SERVICE_NAME = "gov.nasa.worldwind.avkey.ServiceName";
		const std::string SERVICE_NAME_LOCAL_RASTER_SERVER = "LocalRasterServer";
		const std::string SERVICE_NAME_OFFLINE = "Offline";
		const std::string SESSION_CACHE_CLASS_NAME = "gov.nasa.worldwind.avkey.SessionCacheClassName";
		const std::string SHAPE_ATTRIBUTES = "gov.nasa.worldwind.avkey.ShapeAttributes";
		const std::string SHAPE_CIRCLE = "gov.nasa.worldwind.avkey.ShapeCircle";
		const std::string SHAPE_ELLIPSE = "gov.nasa.worldwind.avkey.ShapeEllipse";
		const std::string SHAPE_LINE = "gov.nasa.worldwind.avkey.ShapeLine";
		const std::string SHAPE_NONE = "gov.nasa.worldwind.avkey.ShapeNone";
		const std::string SHAPE_PATH = "gov.nasa.worldwind.avkey.ShapePath";
		const std::string SHAPE_POLYGON = "gov.nasa.worldwind.avkey.ShapePolygon";
		const std::string SHAPE_QUAD = "gov.nasa.worldwind.avkey.ShapeQuad";
		const std::string SHAPE_RECTANGLE = "gov.nasa.worldwind.avkey.ShapeRectangle";
		const std::string SHAPE_SQUARE = "gov.nasa.worldwind.avkey.ShapeSquare";
		const std::string SHAPE_TRIANGLE = "gov.nasa.worldwind.avkey.ShapeTriangle";
		const std::string SHORT_DESCRIPTION = "gov.nasa.worldwind.avkey.Server.ShortDescription";
		const std::string SIZE_FIT_TEXT = "gov.nasa.worldwind.avkey.SizeFitText";
		const std::string SIZE_FIXED = "gov.nasa.worldwind.avkey.SizeFixed";
		const std::string SPATIAL_REFERENCE_WKT = "gov.nasa.worldwind.avkey.SpatialReference.WKT";
		const std::string SOUTH = "gov.nasa.worldwdind.avkey.South";
		const std::string START = "gov.nasa.worldwind.avkey.Start";
		const std::string STEREO_FOCUS_ANGLE = "gov.nasa.worldwind.StereoFocusAngle";
		const std::string STEREO_INTEROCULAR_DISTANCE = "gov.nasa.worldwind.StereoFInterocularDistance";
		const std::string STEREO_MODE = "gov.nasa.worldwind.stereo.mode"; // lowercase to match Java property convention
		const std::string STEREO_MODE_DEVICE = "gov.nasa.worldwind.avkey.StereoModeDevice";
		const std::string STEREO_MODE_NONE = "gov.nasa.worldwind.avkey.StereoModeNone";
		const std::string STEREO_MODE_RED_BLUE = "gov.nasa.worldwind.avkey.StereoModeRedBlue";
		const std::string STEREO_TYPE = "gov.nasa.worldwind.stereo.type";
		const std::string STEREO_TYPE_TOED_IN = "gov.nasa.worldwind.avkey.StereoModeToedIn";
		const std::string STEREO_TYPE_PARALLEL = "gov.nasa.worldwind.avkey.StereoModeParallel";
		const std::string STOP = "gov.nasa.worldwind.avkey.Stop";
		const std::string STYLE_NAMES = "gov.nasa.worldwind.avkey.StyleNames";
		const std::string SURFACE_TILE_DRAW_CONTEXT = "gov.nasa.worldwind.avkey.SurfaceTileDrawContext";

		const std::string TESSELLATOR_CLASS_NAME = "gov.nasa.worldwind.avkey.TessellatorClassName";
		const std::string TEXTURE = "gov.nasa.worldwind.avkey.Texture";
		const std::string TEXTURE_CACHE_SIZE = "gov.nasa.worldwind.avkey.TextureCacheSize";
		const std::string TEXTURE_COORDINATES = "gov.nasa.worldwind.avkey.TextureCoordinates";
		const std::string TEXTURE_FORMAT = "gov.nasa.worldwind.avkey.TextureFormat";
		const std::string TEXTURE_IMAGE_CACHE_SIZE = "gov.nasa.worldwind.avkey.TextureTileCacheSize";
		const std::string TARGET = "gov.nasa.worldwind.avkey.Target";
		const std::string TASK_POOL_SIZE = "gov.nasa.worldwind.avkey.TaskPoolSize";
		const std::string TASK_QUEUE_SIZE = "gov.nasa.worldwind.avkey.TaskQueueSize";
		const std::string TASK_SERVICE_CLASS_NAME = "gov.nasa.worldwind.avkey.TaskServiceClassName";
		const std::string TEXT = "gov.nasa.worldwind.avkey.Text";
		const std::string TEXT_EFFECT_NONE = "gov.nasa.worldwind.avkey.TextEffectNone";
		const std::string TEXT_EFFECT_OUTLINE = "gov.nasa.worldwind.avkey.TextEffectOutline";
		const std::string TEXT_EFFECT_SHADOW = "gov.nasa.worldwind.avkey.TextEffectShadow";
		const std::string TILE_DELTA = "gov.nasa.worldwind.avkey.TileDeltaKey";
		const std::string TILE_HEIGHT = "gov.nasa.worldwind.avkey.TileHeightKey";
		const std::string TILE_ORIGIN = "gov.nasa.worldwind.avkey.TileOrigin";
		const std::string TILE_RETRIEVER = "gov.nasa.worldwind.avkey.TileRetriever";
		const std::string TILE_URL_BUILDER = "gov.nasa.worldwind.avkey.TileURLBuilder";
		const std::string TILE_WIDTH = "gov.nasa.worldwind.avkey.TileWidthKey";
		const std::string TILED_IMAGERY = "gov.nasa.worldwind.avkey.TiledImagery";
		const std::string TILED_ELEVATIONS = "gov.nasa.worldwind.avkey.TiledElevations";
		const std::string TILED_RASTER_PRODUCER_CACHE_SIZE = "gov.nasa.worldwind.avkey.TiledRasterProducerCacheSize";
		const std::string TILED_RASTER_PRODUCER_LARGE_DATASET_THRESHOLD =
			"gov.nasa.worldwind.avkey.TiledRasterProducerLargeDatasetThreshold";
		const std::string TILED_RASTER_PRODUCER_LIMIT_MAX_LEVEL = "gov.nasa.worldwind.avkey.TiledRasterProducer.LimitMaxLevel";
		const std::string TITLE = "gov.nasa.worldwind.avkey.Title";
		const std::string TOP = "gov.nasa.worldwind.avkey.Top";
		const std::string TRANSPARENCY_COLORS = "gov.nasa.worldwind.avkey.TransparencyColors";
		const std::string TREE = "gov.nasa.worldwind.avkey.Tree";
		const std::string TREE_NODE = "gov.nasa.worldwind.avkey.TreeNode";

		const std::string UNIT_FOOT = "gov.nasa.worldwind.avkey.Unit.Foot";
		const std::string UNIT_METER = "gov.nasa.worldwind.avkey.Unit.Meter";

		const std::string UNRESOLVED = "gov.nasa.worldwind.avkey.Unresolved";
		const std::string UPDATED = "gov.nasa.worldwind.avkey.Updated";
		const std::string URL = "gov.nasa.worldwind.avkey.URL";
		const std::string URL_CONNECT_TIMEOUT = "gov.nasa.worldwind.avkey.URLConnectTimeout";
		const std::string URL_PROXY_HOST = "gov.nasa.worldwind.avkey.UrlProxyHost";
		const std::string URL_PROXY_PORT = "gov.nasa.worldwind.avkey.UrlProxyPort";
		const std::string URL_PROXY_TYPE = "gov.nasa.worldwind.avkey.UrlProxyType";
		const std::string URL_READ_TIMEOUT = "gov.nasa.worldwind.avkey.URLReadTimeout";
		const std::string USE_MIP_MAPS = "gov.nasa.worldwind.avkey.UseMipMaps";
		const std::string USE_TRANSPARENT_TEXTURES = "gov.nasa.worldwind.avkey.UseTransparentTextures";

		const std::string VBO_THRESHOLD = "gov.nasa.worldwind.avkey.VBOThreshold";
		const std::string VBO_USAGE = "gov.nasa.worldwind.avkey.VBOUsage";
		const std::string VERSION = "gov.nasa.worldwind.avkey.Version";
		const std::string VERTICAL = "gov.nasa.worldwind.avkey.Vertical";
		const std::string VERTICAL_EXAGGERATION = "gov.nasa.worldwind.avkey.VerticalExaggeration";
		const std::string VERTICAL_EXAGGERATION_UP = "gov.nasa.worldwind.avkey.VerticalExaggerationUp";
		const std::string VERTICAL_EXAGGERATION_DOWN = "gov.nasa.worldwind.avkey.VerticalExaggerationDown";
		const std::string VIEW = "gov.nasa.worldwind.avkey.ViewObject";
		const std::string VIEW_CLASS_NAME = "gov.nasa.worldwind.avkey.ViewClassName";
		const std::string VIEW_INPUT_HANDLER_CLASS_NAME = "gov.nasa.worldwind.avkey.ViewInputHandlerClassName";
		const std::string VIEW_QUIET = "gov.nasa.worldwind.avkey.ViewQuiet";

		// Viewing operations
		const std::string VIEW_OPERATION = "gov.nasa.worldwind.avkey.ViewOperation";
		const std::string VIEW_PAN = "gov.nasa.worldwind.avkey.Pan";
		const std::string VIEW_LOOK = "gov.nasa.worldwind.avkey.ControlLook";
		const std::string VIEW_HEADING_LEFT = "gov.nasa.worldwind.avkey.HeadingLeft";
		const std::string VIEW_HEADING_RIGHT = "gov.nasa.worldwind.avkey.HeadingRight";
		const std::string VIEW_ZOOM_IN = "gov.nasa.worldwind.avkey.ZoomIn";
		const std::string VIEW_ZOOM_OUT = "gov.nasa.worldwind.avkey.ZoomOut";
		const std::string VIEW_PITCH_UP = "gov.nasa.worldwind.avkey.PitchUp";
		const std::string VIEW_PITCH_DOWN = "gov.nasa.worldwind.avkey.PitchDown";
		const std::string VIEW_FOV_NARROW = "gov.nasa.worldwind.avkey.FovNarrow";
		const std::string VIEW_FOV_WIDE = "gov.nasa.worldwind.avkey.FovWide";

		const std::string VISIBILITY_ACTION_RELEASE = "gov.nasa.worldwind.avkey.VisibilityActionRelease";
		const std::string VISIBILITY_ACTION_RETAIN = "gov.nasa.worldwind.avkey.VisibilityActionRetain";

		const std::string WAKEUP_TIMEOUT = "gov.nasa.worldwind.avkey.WakeupTimeout";
		const std::string WEB_VIEW_FACTORY = "gov.nasa.worldwind.avkey.WebViewFactory";
		const std::string WEST = "gov.nasa.worldwind.avkey.West";
		const std::string WIDTH = "gov.nasa.worldwind.avkey.Width";
		const std::string WMS_BACKGROUND_COLOR = "gov.nasa.worldwind.avkey.BackgroundColor";

		const std::string WFS_URL = "gov.nasa.worldwind.avkey.WFS.URL";
		const std::string WMS_VERSION = "gov.nasa.worldwind.avkey.WMSVersion";
		const std::string WORLD_MAP_IMAGE_PATH = "gov.nasa.worldwind.avkey.WorldMapImagePath";
		const std::string WORLD_WIND_DOT_NET_LAYER_SET = "gov.nasa.worldwind.avkey.WorldWindDotNetLayerSet";
		const std::string WORLD_WIND_DOT_NET_PERMANENT_DIRECTORY = "gov.nasa.worldwind.avkey.WorldWindDotNetPermanentDirectory";
		const std::string WORLD_WINDOW_CLASS_NAME = "gov.nasa.worldwind.avkey.WorldWindowClassName";
		const std::string MODEL_STORE_PATH = "gov.nasa.worldwind.platform.alluser.Store";
	}

	class ElevationConfig 
	{
	public:

		using Key = std::string;

		enum class DataType {
			Float32,
			Float64,
			Int16,
			Int32,
		};

		enum class ByteOrder {
			LittleEndian,
			BigEndian,
		};

		using Value = Variant<	float, 
								int, 
								char, 
								std::string, 
								LatLon, 
								Sector,
								DataType,
								ByteOrder,
								std::pair<int, int>>;

	public:

		ElevationConfig(const std::string & xmlFiles);

		template <typename T>
		bool GetValue(const Key & key, T *value) const 
		{
			if (value)
			{
				auto result = map_.find(key);
				if (result != map_.end())
				{
					try
					{
						*value = result->second.Get<T>();
						return true;
					}
					catch (const std::bad_variant_access & )
					{
					}
				}
			}
			return false;
		}

		template <typename T>
		void SetValue(const Key & key, const T & value) 
		{
			map_[key] = Value{ value };
		}

	private:


		std::unordered_map<Key, Value> map_;
	
	};

}