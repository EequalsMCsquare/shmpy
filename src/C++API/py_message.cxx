#include "py_message.hpp"

#include "py_variable.hpp"
namespace shmpy {
RESP_VariableShmInsert::RESP_VariableShmInsert(sptr<variable_desc> var_desc)
{
  this->ec      = ShmpyErrc::NoError;
  this->segment = var_desc->get_segment_info();
  this->dtype   = var_desc->dtype_;
  this->is_bp   = var_desc->is_bp_;
  this->size    = var_desc->size_;
}

RESP_VariableShmInsert::RESP_VariableShmInsert(ShmpyErrc ec)
{
  this->ec = ec;
}

RESP_VariableShmInsert::RESP_VariableShmInsert(sptr<base_segment_t> segment,
                                               const DTYPE          dtype,
                                               const bool           is_bp,
                                               const size_t         size)
{
  this->ec      = ShmpyErrc::NoError;
  this->segment = segment->to_seginfo();
  this->dtype   = dtype;
  this->is_bp   = is_bp;
  this->size    = size;
}
}