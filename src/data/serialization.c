#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/serialization.h"
#include "data/data_store.h"
#include "utils/logger.h"

CQError serialize_to_json(const char *filepath)
{
    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    return data_store_serialize_json(filepath);
}

CQError deserialize_from_json(const char *filepath)
{
    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // JSON deserialization is more complex and would require a JSON parser
    // For now, we'll only support binary format for loading cached results
    LOG_WARNING("JSON deserialization not implemented - use binary format for caching");
    return CQ_ERROR_UNKNOWN;
}

CQError export_to_csv(const char *filepath)
{
    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    return data_store_export_csv(filepath);
}

CQError save_binary_results(const char *filepath)
{
    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    return data_store_serialize_binary(filepath);
}

CQError load_binary_results(const char *filepath)
{
    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    return data_store_deserialize_binary(filepath);
}
