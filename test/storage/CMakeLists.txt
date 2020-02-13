#if(WITH_ROCKSDB)
#    addtest(rocks_storage_test rocks_storage_test.cpp)
#    target_link_libraries(rocks_storage_test vbkutils)
#endif()

addtest(storage_acceptance_test storage_acceptance_test.cpp)
target_link_libraries(storage_acceptance_test vbkutils rocksdb)