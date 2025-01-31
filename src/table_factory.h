#pragma once

#include <atomic>

#include "rocksdb/table.h"

#include "blob_file_manager.h"
#include "blob_file_set.h"
#include "titan/options.h"
#include "titan_stats.h"

namespace rocksdb {
namespace titandb {

class TitanDBImpl;

class TitanTableFactory : public TableFactory {
 public:
  TitanTableFactory(const TitanDBOptions& db_options,
                    const TitanCFOptions& cf_options,
                    std::shared_ptr<BlobFileManager> blob_manager,
                    port::Mutex* db_mutex, BlobFileSet* blob_file_set,
                    TitanStats* stats)
      : db_options_(db_options),
        cf_options_(cf_options),
        blob_run_mode_(cf_options.blob_run_mode),
        base_factory_(cf_options.table_factory),
        blob_manager_(blob_manager),
        db_mutex_(db_mutex),
        blob_file_set_(blob_file_set),
        stats_(stats) {}

  const char* Name() const override { return "TitanTable"; }

  using TableFactory::NewTableReader;

  Status NewTableReader(
      const ReadOptions& ro, const TableReaderOptions& options,
      std::unique_ptr<RandomAccessFileReader>&& file, uint64_t file_size,
      std::unique_ptr<TableReader>* result,
      bool prefetch_index_and_filter_in_cache = true) const override;

  TableBuilder* NewTableBuilder(const TableBuilderOptions& options,
                                WritableFileWriter* file) const override;

  void SetBlobRunMode(TitanBlobRunMode mode) { blob_run_mode_.store(mode); }

  bool IsDeleteRangeSupported() const override {
    return base_factory_->IsDeleteRangeSupported();
  }

 private:
  const TitanDBOptions db_options_;
  const TitanCFOptions cf_options_;
  std::atomic<TitanBlobRunMode> blob_run_mode_;
  std::shared_ptr<TableFactory> base_factory_;
  std::shared_ptr<BlobFileManager> blob_manager_;
  port::Mutex* db_mutex_;
  BlobFileSet* blob_file_set_;
  TitanStats* stats_;
};

}  // namespace titandb
}  // namespace rocksdb
