if(WITH_ROCKSDB)
    addtest(storage_test storage_test.cpp)
    target_link_libraries(storage_test rocksdb)
    set_target_properties(storage_test PROPERTIES
            RESOURCE_LOCK db-test
            )

    addtest(rocks_storage_test rocks_storage_test.cpp)
    target_link_libraries(rocks_storage_test rocksdb)
    set_target_properties(rocks_storage_test PROPERTIES
            RESOURCE_LOCK db-test
            )
    addtest(endorsement_storage_test endorsement_storage_test.cpp)
    target_link_libraries(endorsement_storage_test rocksdb)
    set_target_properties(endorsement_storage_test PROPERTIES
            RESOURCE_LOCK db-test
            )
            
    addtest(payloads_storage_test payloads_storage_test.cpp)
    target_link_libraries(payloads_storage_test rocksdb)
    set_target_properties(payloads_storage_test PROPERTIES
            RESOURCE_LOCK db-test
            )
endif()