#include <regex>
#include "api.h"
#include "collection.h"
#include "collection_manager.h"

void get_search(http_req & req, http_res & res) {
    const char *NUM_TYPOS = "num_typos";
    const char *PREFIX = "prefix";
    const char *TOKEN_ORDERING = "token_ordering";
    const char *FILTERS = "filters";

    if(req.params.count(NUM_TYPOS) == 0) {
        req.params[NUM_TYPOS] = "2";
    }

    if(req.params.count(PREFIX) == 0) {
        req.params[PREFIX] = "false";
    }

    if(req.params.count(TOKEN_ORDERING) == 0) {
        req.params[TOKEN_ORDERING] = "FREQUENCY";
    }

    std::string filter_str = req.params.count(FILTERS) != 0 ? req.params[FILTERS] : "";
    //std::cout << "filter_str: " << filter_str << std::endl;

    token_ordering token_order = (req.params[TOKEN_ORDERING] == "MAX_SCORE") ? MAX_SCORE : FREQUENCY;

    //printf("Query: %s\n", req.params["q"].c_str());
    auto begin = std::chrono::high_resolution_clock::now();

    std::vector<std::string> search_fields = {"title"};

    CollectionManager & collectionManager = CollectionManager::get_instance();
    Collection* collection = collectionManager.get_collection(req.params["collection"]);

    if(collection == nullptr) {
        return res.send_404();
    }

    nlohmann::json result = collection->search(req.params["q"], search_fields, filter_str, { },
                                               {"points"}, std::stoi(req.params[NUM_TYPOS]), 100, token_order, false);
    const std::string & json_str = result.dump();
    //std::cout << "JSON:" << json_str << std::endl;
    struct rusage r_usage;
    getrusage(RUSAGE_SELF,&r_usage);

    //std::cout << "Memory usage: " << r_usage.ru_maxrss << std::endl;
    res.send_200(json_str);

    long long int timeMillis = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();
    std::cout << "Time taken: " << timeMillis << "us" << std::endl;
}

void post_add_document(http_req & req, http_res & res) {
    CollectionManager & collectionManager = CollectionManager::get_instance();
    Collection* collection = collectionManager.get_collection(req.params["collection"]);

    if(collection == nullptr) {
        return res.send_404();
    }

    Option<std::string> inserted_id_op = collection->add(req.body);

    nlohmann::json json_response;
    static h2o_generator_t generator = {NULL, NULL};

    if(!inserted_id_op.ok()) {
        json_response["message"] = inserted_id_op.error();
        res.send_500(json_response.dump());

    } else {
        json_response["id"] = inserted_id_op.get();
        res.send_201(json_response.dump());
    }
}

/*
int del_remove_document(h2o_handler_t *self, h2o_req_t *req) {
    h2o_iovec_t query = req->query_at != SIZE_MAX ?
                        h2o_iovec_init(req->path.base + req->query_at, req->path.len - req->query_at) :
                        h2o_iovec_init(H2O_STRLIT(""));

    std::string query_str(query.base, query.len);
    std::map<std::string, std::string> req.params = parse_query(query_str);

    std::string doc_id = req.params["id"];

    auto begin = std::chrono::high_resolution_clock::now();
    collection->remove(doc_id);
    long long int time_micro = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - begin).count();
    std::cout << "Time taken: " << time_micro << "us" << std::endl;

    nlohmann::json json_response;
    json_response["id"] = doc_id;
    json_response["status"] = "SUCCESS";

    static h2o_generator_t generator = {NULL, NULL};
    req->res.status = 200;
    req->res.reason = "OK";
    h2o_add_header(&req->pool, &req->res.headers, H2O_TOKEN_CONTENT_TYPE, H2O_STRLIT("application/json; charset=utf-8"));
    h2o_start_response(req, &generator);
    h2o_iovec_t body = h2o_strdup(&req->pool, json_response.dump().c_str(), SIZE_MAX);
    h2o_send(req, &body, 1, 1);
    return 0;
}
*/