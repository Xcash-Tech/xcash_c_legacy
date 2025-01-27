#include <inttypes.h>
#include "wallet2_api_c.h"
#include <unistd.h>
#include "helpers.hpp"
#include <cstring>
#include <thread>
#include "../../../../xcash/src/wallet/api/wallet2_api.h"
#include "xcash_checksum.h"

#ifdef __cplusplus
extern "C"
{
#endif


// The code in here consists of simple wrappers, that convert
// more advanced c++ types (and function names) into simple C-compatible
// functions, so these implementations can be easly used from all languages
// that do support C interop (such as dart)
//
//
// Here is the most complex definition that we can find in the current codebase, it even includes
// a if statement - which in general I consider an anti-patter in just wrappers
//
//  _____________ void* because C++ wallet->createTransaction returns a pointer to XCash::PendingTransaction, which we don't want to have exposed in C land
// /      _____________ XCASH prefix just means that this function is using xcash codebase, to not cause any symbols collision when using more than one libwallet2_api_c.so in a single program.
// |     /       _____________ Wallet is one of the classes in Xcash namespace in the upstream codebase (see the include line above)
// |     |      /       _____________ aaand it is calling createTransaction function.
// |     |      |      /                  _________________________________________________________________________________
// |     |      |      |                 /                                                                                 \ All of these parameters can be found in the upstream
// |     |      |      |                |                                                                     _____________/ function definition, if something was more complex -
// void* XCASH_Wallet_createTransaction(void* wallet_ptr, const char* dst_addr, const char* payment_id,     / like std::set I've used splitString functions and introduced a new
//                                                     uint64_t amount, uint32_t mixin_count,               / parameter - separator, as it is the simplest way to get vector onto
//                                                     int pendingTransactionPriority,                     / C side from more advanced world.
//                                                     uint32_t subaddr_account,                          /
//                                                     const char* preferredInputs, const char* separator) {
//     XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr); <------------ We are converting the void* into XCash::Wallet*
//     XCash::optional<uint64_t> optAmount; <------------- optional by default
//     if (amount != 0) {------------------\ We set this optional parameter only when it isn't zero
//         optAmount = amount;             | 
//     }___________________________________/
//     std::set<uint32_t> subaddr_indices = {}; ------------- Default value
//     std::set<std::string> preferred_inputs = splitString(std::string(preferredInputs), std::string(separator)); <------------- We are using helpers.cpp function to split a string into std::set
//     return wallet->createTransaction(std::string(dst_addr), std::string(payment_id),-\ const char * is getting casted onto std::string
//                                         optAmount, mixin_count,        \_____________/
//                                         PendingTransaction_Priority_fromInt(pendingTransactionPriority), <------------- special case for this function to get native type instead of int value.
//                                         subaddr_account, subaddr_indices, preferred_inputs);
// }
//
//
// One case which is not covered here is when we have to return a string
// const char* XCASH_PendingTransaction_errorString(void* pendingTx_ptr) {
//     XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
//     std::string str = pendingTx->errorString(); <------------- get the actual string from the upstream codebase
//     const std::string::size_type size = str.size(); ------------------------------\
//     char *buffer = new char[size + 1];   //we need extra char for NUL             | Copy the string onto a new memory so it won't get freed after the function returns
//     memcpy(buffer, str.c_str(), size + 1);                                        | NOTE: This requires us to call free() after we are done with the text processing
//     return buffer; ______________________________________________________________/
// }
//
//

// PendingTransaction

int XCASH_PendingTransaction_status(void* pendingTx_ptr) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    return pendingTx->status();
}
const char* XCASH_PendingTransaction_errorString(void* pendingTx_ptr) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    std::string str = pendingTx->errorString();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
bool XCASH_PendingTransaction_commit(void* pendingTx_ptr, const char* filename, bool overwrite) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    return pendingTx->commit(std::string(filename), overwrite);
}
const char* XCASH_PendingTransaction_commitUR(void* pendingTx_ptr, int max_fragment_length) {
    throw std::runtime_error("XCASH_PendingTransaction_commitUR Not implemented");
    // XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    // std::string str = pendingTx->commitUR(max_fragment_length);
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}
uint64_t XCASH_PendingTransaction_amount(void* pendingTx_ptr) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    return pendingTx->amount();
}
uint64_t XCASH_PendingTransaction_dust(void* pendingTx_ptr) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    return pendingTx->dust();
}
uint64_t XCASH_PendingTransaction_fee(void* pendingTx_ptr) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    return pendingTx->fee();
}
const char* XCASH_PendingTransaction_txid(void* pendingTx_ptr, const char* separator) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    std::vector<std::string> txid = pendingTx->txid();
    return vectorToString(txid, std::string(separator));
}
uint64_t XCASH_PendingTransaction_txCount(void* pendingTx_ptr) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    return pendingTx->txCount();
}
const char* XCASH_PendingTransaction_subaddrAccount(void* pendingTx_ptr, const char* separator) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    std::vector<uint32_t> subaddrAccount = pendingTx->subaddrAccount();
    return vectorToString(subaddrAccount, std::string(separator));
}
const char* XCASH_PendingTransaction_subaddrIndices(void* pendingTx_ptr, const char* separator) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    std::vector<std::set<uint32_t>> subaddrIndices = pendingTx->subaddrIndices();
    return vectorToString(subaddrIndices, std::string(separator));
}
const char* XCASH_PendingTransaction_multisigSignData(void* pendingTx_ptr) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    std::string str = pendingTx->multisigSignData();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
void XCASH_PendingTransaction_signMultisigTx(void* pendingTx_ptr) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    return pendingTx->signMultisigTx();
}
const char* XCASH_PendingTransaction_signersKeys(void* pendingTx_ptr, const char* separator) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    std::vector<std::string> txid = pendingTx->signersKeys();
    return vectorToString(txid, std::string(separator));
}

const char* XCASH_PendingTransaction_hex(void* pendingTx_ptr, const char* separator) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    std::vector<std::string> txid = pendingTx->hex();
    return vectorToString(txid, std::string(separator));
}

const char* XCASH_PendingTransaction_txKey(void* pendingTx_ptr, const char* separator) {
    XCash::PendingTransaction *pendingTx = reinterpret_cast<XCash::PendingTransaction*>(pendingTx_ptr);
    std::vector<std::string> txid = pendingTx->txKey();
    return vectorToString(txid, std::string(separator));
}

// UnsignedTransaction

int XCASH_UnsignedTransaction_status(void* unsignedTx_ptr) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return unsignedTx->status();
}
const char* XCASH_UnsignedTransaction_errorString(void* unsignedTx_ptr) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    std::string str = unsignedTx->errorString();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
const char* XCASH_UnsignedTransaction_amount(void* unsignedTx_ptr, const char* separator) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return vectorToString(unsignedTx->amount(), std::string(separator));
}
const char* XCASH_UnsignedTransaction_fee(void* unsignedTx_ptr, const char* separator) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return vectorToString(unsignedTx->fee(), std::string(separator));
}
const char* XCASH_UnsignedTransaction_mixin(void* unsignedTx_ptr, const char* separator) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return vectorToString(unsignedTx->mixin(), std::string(separator));
}
const char* XCASH_UnsignedTransaction_confirmationMessage(void* unsignedTx_ptr) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    std::string str = unsignedTx->confirmationMessage();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
const char* XCASH_UnsignedTransaction_paymentId(void* unsignedTx_ptr, const char* separator) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return vectorToString(unsignedTx->paymentId(), std::string(separator));
}
const char* XCASH_UnsignedTransaction_recipientAddress(void* unsignedTx_ptr, const char* separator) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return vectorToString(unsignedTx->recipientAddress(), std::string(separator));
}
uint64_t XCASH_UnsignedTransaction_minMixinCount(void* unsignedTx_ptr) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return unsignedTx->minMixinCount();
}
uint64_t XCASH_UnsignedTransaction_txCount(void* unsignedTx_ptr) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return unsignedTx->txCount();
}
bool XCASH_UnsignedTransaction_sign(void* unsignedTx_ptr, const char* signedFileName) {
    XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    return unsignedTx->sign(std::string(signedFileName));
}
const char* XCASH_UnsignedTransaction_signUR(void* unsignedTx_ptr, int max_fragment_length) {
    throw std::runtime_error("XCASH_UnsignedTransaction_signUR Not implemented");
    // XCash::UnsignedTransaction *unsignedTx = reinterpret_cast<XCash::UnsignedTransaction*>(unsignedTx_ptr);
    // std::string str = unsignedTx->signUR(max_fragment_length);
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}
// TransactionInfo
int XCASH_TransactionInfo_direction(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->direction();
}
bool XCASH_TransactionInfo_isPending(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->isPending();
}
bool XCASH_TransactionInfo_isFailed(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->isFailed();
}
bool XCASH_TransactionInfo_isCoinbase(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->isCoinbase();
}
uint64_t XCASH_TransactionInfo_amount(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->amount();
}
uint64_t XCASH_TransactionInfo_fee(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->fee();
}
uint64_t XCASH_TransactionInfo_blockHeight(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->blockHeight();
}
const char* XCASH_TransactionInfo_description(void* txInfo_ptr) {
    throw std::runtime_error("XCASH_TransactionInfo_description Not implemented");
    // XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    // std::string str = txInfo->description();
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}
const char* XCASH_TransactionInfo_subaddrIndex(void* txInfo_ptr, const char* separator) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    std::set<uint32_t> subaddrIndex = txInfo->subaddrIndex();
    return vectorToString(subaddrIndex, std::string(separator));
}
uint32_t XCASH_TransactionInfo_subaddrAccount(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->subaddrAccount();
}
const char* XCASH_TransactionInfo_label(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    std::string str = txInfo->label();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
uint64_t XCASH_TransactionInfo_confirmations(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->confirmations();
}
uint64_t XCASH_TransactionInfo_unlockTime(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->unlockTime();
}
const char* XCASH_TransactionInfo_hash(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    std::string str = txInfo->hash();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
uint64_t XCASH_TransactionInfo_timestamp(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->timestamp();
}
const char* XCASH_TransactionInfo_paymentId(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    std::string str = txInfo->paymentId();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

int XCASH_TransactionInfo_transfers_count(void* txInfo_ptr) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->transfers().size();
}

uint64_t XCASH_TransactionInfo_transfers_amount(void* txInfo_ptr, int index) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    return txInfo->transfers()[index].amount;
}

const char* XCASH_TransactionInfo_transfers_address(void* txInfo_ptr, int index) {
    XCash::TransactionInfo *txInfo = reinterpret_cast<XCash::TransactionInfo*>(txInfo_ptr);
    std::string str = txInfo->transfers()[index].address;
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}




// TransactionHistory
int XCASH_TransactionHistory_count(void* txHistory_ptr) {
    XCash::TransactionHistory *txHistory = reinterpret_cast<XCash::TransactionHistory*>(txHistory_ptr);
    return txHistory->count();
}
void* XCASH_TransactionHistory_transaction(void* txHistory_ptr, int index) {
    XCash::TransactionHistory *txHistory = reinterpret_cast<XCash::TransactionHistory*>(txHistory_ptr);
    return reinterpret_cast<void*>(txHistory->transaction(index));
}
void* XCASH_TransactionHistory_transactionById(void* txHistory_ptr, const char* id) {
    XCash::TransactionHistory *txHistory = reinterpret_cast<XCash::TransactionHistory*>(txHistory_ptr);
    return reinterpret_cast<void*>(txHistory->transaction(std::string(id)));
}

void XCASH_TransactionHistory_refresh(void* txHistory_ptr) {
    XCash::TransactionHistory *txHistory = reinterpret_cast<XCash::TransactionHistory*>(txHistory_ptr);
    return txHistory->refresh();
}
void XCASH_TransactionHistory_setTxNote(void* txHistory_ptr, const char* txid, const char* note) {
    throw std::runtime_error("XCASH_TransactionHistory_setTxNote Not implemented");
    // XCash::TransactionHistory *txHistory = reinterpret_cast<XCash::TransactionHistory*>(txHistory_ptr);
    // return txHistory->setTxNote(std::string(txid), std::string(note));
}

// AddressBokRow

//     std::string extra;
const char* XCASH_AddressBookRow_extra(void* addressBookRow_ptr) {
    XCash::AddressBookRow *addressBookRow = reinterpret_cast<XCash::AddressBookRow*>(addressBookRow_ptr);
    std::string str = addressBookRow->extra;
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getAddress() const {return m_address;} 
const char* XCASH_AddressBookRow_getAddress(void* addressBookRow_ptr) {
    XCash::AddressBookRow *addressBookRow = reinterpret_cast<XCash::AddressBookRow*>(addressBookRow_ptr);
    std::string str = addressBookRow->getAddress();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getDescription() const {return m_description;} 
const char* XCASH_AddressBookRow_getDescription(void* addressBookRow_ptr) {
    XCash::AddressBookRow *addressBookRow = reinterpret_cast<XCash::AddressBookRow*>(addressBookRow_ptr);
    std::string str = addressBookRow->getDescription();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getPaymentId() const {return m_paymentId;} 
const char* XCASH_AddressBookRow_getPaymentId(void* addressBookRow_ptr) {
    XCash::AddressBookRow *addressBookRow = reinterpret_cast<XCash::AddressBookRow*>(addressBookRow_ptr);
    std::string str = addressBookRow->getPaymentId();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::size_t getRowId() const {return m_rowId;}
size_t XCASH_AddressBookRow_getRowId(void* addressBookRow_ptr) {
    XCash::AddressBookRow *addressBookRow = reinterpret_cast<XCash::AddressBookRow*>(addressBookRow_ptr);
    return addressBookRow->getRowId();
}

// AddressBook
//     virtual std::vector<AddressBookRow*> getAll() const = 0;
int XCASH_AddressBook_getAll_size(void* addressBook_ptr) {
    XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    return addressBook->getAll().size();
}
void* XCASH_AddressBook_getAll_byIndex(void* addressBook_ptr, int index) {
    XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    return addressBook->getAll()[index];
}
//     virtual bool addRow(const std::string &dst_addr , const std::string &payment_id, const std::string &description) = 0;  
bool XCASH_AddressBook_addRow(void* addressBook_ptr, const char* dst_addr , const char* payment_id, const char* description) {
    XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    return addressBook->addRow(std::string(dst_addr), std::string(payment_id), std::string(description));
}
//     virtual bool deleteRow(std::size_t rowId) = 0;
bool XCASH_AddressBook_deleteRow(void* addressBook_ptr, size_t rowId) {
    XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    return addressBook->deleteRow(rowId);
}
//     virtual bool setDescription(std::size_t index, const std::string &description) = 0;
bool XCASH_AddressBook_setDescription(void* addressBook_ptr, size_t rowId, const char* description) {
    throw std::runtime_error("XCASH_AddressBook_setDescription Not implemented");
    // XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    // return addressBook->setDescription(rowId, std::string(description));
}
//     virtual void refresh() = 0;  
void XCASH_AddressBook_refresh(void* addressBook_ptr) {
    XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    return addressBook->refresh();
}
//     virtual std::string errorString() const = 0;
const char* XCASH_AddressBook_errorString(void* addressBook_ptr) {
    XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    std::string str = addressBook->errorString();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     virtual int errorCode() const = 0;
int XCASH_AddressBook_errorCode(void* addressBook_ptr) {
    XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    return addressBook->errorCode();
}
//     virtual int lookupPaymentID(const std::string &payment_id) const = 0;
int XCASH_AddressBook_lookupPaymentID(void* addressBook_ptr, const char* payment_id) {
    XCash::AddressBook *addressBook = reinterpret_cast<XCash::AddressBook*>(addressBook_ptr);
    return addressBook->lookupPaymentID(std::string(payment_id));
}

// CoinsInfo
uint64_t XCASH_CoinsInfo_blockHeight(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->blockHeight();
}
//     virtual std::string hash() const = 0;
const char* XCASH_CoinsInfo_hash(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    std::string str = coinsInfo->hash();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     virtual size_t internalOutputIndex() const = 0;
size_t XCASH_CoinsInfo_internalOutputIndex(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->internalOutputIndex();
}
//     virtual uint64_t globalOutputIndex() const = 0;
uint64_t XCASH_CoinsInfo_globalOutputIndex(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->globalOutputIndex();
}
//     virtual bool spent() const = 0;
bool XCASH_CoinsInfo_spent(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->spent();
}
//     virtual bool frozen() const = 0;
bool XCASH_CoinsInfo_frozen(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->frozen();
}
//     virtual uint64_t spentHeight() const = 0;
uint64_t XCASH_CoinsInfo_spentHeight(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->spentHeight();
}
//     virtual uint64_t amount() const = 0;
uint64_t XCASH_CoinsInfo_amount(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->amount();
}
//     virtual bool rct() const = 0;
bool XCASH_CoinsInfo_rct(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->rct();
}
//     virtual bool keyImageKnown() const = 0;
bool XCASH_CoinsInfo_keyImageKnown(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->keyImageKnown();
}
//     virtual size_t pkIndex() const = 0;
size_t XCASH_CoinsInfo_pkIndex(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->pkIndex();
}
//     virtual uint32_t subaddrIndex() const = 0;
uint32_t XCASH_CoinsInfo_subaddrIndex(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->subaddrIndex();
}
//     virtual uint32_t subaddrAccount() const = 0;
uint32_t XCASH_CoinsInfo_subaddrAccount(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->subaddrAccount();
}
//     virtual std::string address() const = 0;
const char* XCASH_CoinsInfo_address(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    std::string str = coinsInfo->address();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     virtual std::string addressLabel() const = 0;
const char* XCASH_CoinsInfo_addressLabel(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    std::string str = coinsInfo->addressLabel();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     virtual std::string keyImage() const = 0;
const char* XCASH_CoinsInfo_keyImage(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    std::string str = coinsInfo->keyImage();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     virtual uint64_t unlockTime() const = 0;
uint64_t XCASH_CoinsInfo_unlockTime(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->unlockTime();
}
//     virtual bool unlocked() const = 0;
bool XCASH_CoinsInfo_unlocked(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->unlocked();
}
//     virtual std::string pubKey() const = 0;
const char* XCASH_CoinsInfo_pubKey(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    std::string str = coinsInfo->pubKey();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     virtual bool coinbase() const = 0;
bool XCASH_CoinsInfo_coinbase(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    return coinsInfo->coinbase();
}
//     virtual std::string description() const = 0;
const char* XCASH_CoinsInfo_description(void* coinsInfo_ptr) {
    XCash::CoinsInfo *coinsInfo = reinterpret_cast<XCash::CoinsInfo*>(coinsInfo_ptr);
    std::string str = coinsInfo->description();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}


// coins

//     virtual ~Coins() = 0;
//     virtual int count() const = 0;
int XCASH_Coins_count(void* coins_ptr) {
    XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    return coins->count();
}
//     virtual CoinsInfo * coin(int index)  const = 0;
void* XCASH_Coins_coin(void* coins_ptr, int index) {
    XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    return coins->coin(index);
}

int XCASH_Coins_getAll_size(void* coins_ptr)  {
    XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    return coins->getAll().size();
}
void* XCASH_Coins_getAll_byIndex(void* coins_ptr, int index) {
    XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    return coins->getAll()[index];
}

//     virtual std::vector<CoinsInfo*> getAll() const = 0;
//     virtual void refresh() = 0;
void XCASH_Coins_refresh(void* coins_ptr) {
    XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    return coins->refresh();
}
//     virtual void setFrozen(std::string public_key) = 0;
void XCASH_Coins_setFrozenByPublicKey(void* coins_ptr, const char* public_key) {
    throw std::runtime_error("XCASH_Coins_setFrozenByPublicKey Not implemented");
    // XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    // return coins->setFrozen(std::string(public_key));
}
//     virtual void setFrozen(int index) = 0;
void XCASH_Coins_setFrozen(void* coins_ptr, int index) {
    XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    return coins->setFrozen(index);
}
//     virtual void thaw(int index) = 0;
void XCASH_Coins_thaw(void* coins_ptr, int index) {
    XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    return coins->thaw(index);
}
//     virtual void thaw(std::string public_key) = 0;
void XCASH_Coins_thawByPublicKey(void* coins_ptr, const char* public_key) {
    throw std::runtime_error("XCASH_Coins_thawByPublicKey Not implemented");
    // XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    // return coins->thaw(std::string(public_key));
}
//     virtual bool isTransferUnlocked(uint64_t unlockTime, uint64_t blockHeight) = 0;
bool XCASH_Coins_isTransferUnlocked(void* coins_ptr, uint64_t unlockTime, uint64_t blockHeight) {
    XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    return coins->isTransferUnlocked(unlockTime, blockHeight);
}
//    virtual void setDescription(const std::string &public_key, const std::string &description) = 0;
void XCASH_Coins_setDescription(void* coins_ptr, const char* public_key, const char* description) {
    throw std::runtime_error("XCASH_Coins_setDescription Not implemented");
    // XCash::Coins *coins = reinterpret_cast<XCash::Coins*>(coins_ptr);
    // coins->setDescription(std::string(public_key), std::string(description));
}

// SubaddressRow

//     std::string extra;
const char* XCASH_SubaddressRow_extra(void* subaddressRow_ptr) {
    XCash::SubaddressRow *subaddressRow = reinterpret_cast<XCash::SubaddressRow*>(subaddressRow_ptr);
    std::string str = subaddressRow->extra;
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getAddress() const {return m_address;}
const char* XCASH_SubaddressRow_getAddress(void* subaddressRow_ptr) {
    XCash::SubaddressRow *subaddressRow = reinterpret_cast<XCash::SubaddressRow*>(subaddressRow_ptr);
    std::string str = subaddressRow->getAddress();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getLabel() const {return m_label;}
const char* XCASH_SubaddressRow_getLabel(void* subaddressRow_ptr) {
    XCash::SubaddressRow *subaddressRow = reinterpret_cast<XCash::SubaddressRow*>(subaddressRow_ptr);
    std::string str = subaddressRow->getLabel();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::size_t getRowId() const {return m_rowId;}
size_t XCASH_SubaddressRow_getRowId(void* subaddressRow_ptr) {
    XCash::SubaddressRow *subaddressRow = reinterpret_cast<XCash::SubaddressRow*>(subaddressRow_ptr);
    return subaddressRow->getRowId();
}

// Subaddress

int XCASH_Subaddress_getAll_size(void* subaddress_ptr) {
    XCash::Subaddress *subaddress = reinterpret_cast<XCash::Subaddress*>(subaddress_ptr);
    return subaddress->getAll().size();
}
void* XCASH_Subaddress_getAll_byIndex(void* subaddress_ptr, int index) {
    XCash::Subaddress *subaddress = reinterpret_cast<XCash::Subaddress*>(subaddress_ptr);
    return subaddress->getAll()[index];
}
//     virtual void addRow(uint32_t accountIndex, const std::string &label) = 0;
void XCASH_Subaddress_addRow(void* subaddress_ptr, uint32_t accountIndex, const char* label) {
    XCash::Subaddress *subaddress = reinterpret_cast<XCash::Subaddress*>(subaddress_ptr);
    return subaddress->addRow(accountIndex, std::string(label));
}
//     virtual void setLabel(uint32_t accountIndex, uint32_t addressIndex, const std::string &label) = 0;
void XCASH_Subaddress_setLabel(void* subaddress_ptr, uint32_t accountIndex, uint32_t addressIndex, const char* label) {
    XCash::Subaddress *subaddress = reinterpret_cast<XCash::Subaddress*>(subaddress_ptr);
    return subaddress->setLabel(accountIndex, addressIndex, std::string(label));
}
//     virtual void refresh(uint32_t accountIndex) = 0;
void XCASH_Subaddress_refresh(void* subaddress_ptr, uint32_t accountIndex) {
    XCash::Subaddress *subaddress = reinterpret_cast<XCash::Subaddress*>(subaddress_ptr);
    return subaddress->refresh(accountIndex);
}

// SubaddressAccountRow

//     std::string extra;
const char* XCASH_SubaddressAccountRow_extra(void* subaddressAccountRow_ptr) {
    XCash::SubaddressAccountRow *subaddressAccountRow = reinterpret_cast<XCash::SubaddressAccountRow*>(subaddressAccountRow_ptr);
    std::string str = subaddressAccountRow->extra;
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getAddress() const {return m_address;}
const char* XCASH_SubaddressAccountRow_getAddress(void* subaddressAccountRow_ptr) {
    XCash::SubaddressAccountRow *subaddressAccountRow = reinterpret_cast<XCash::SubaddressAccountRow*>(subaddressAccountRow_ptr);
    std::string str = subaddressAccountRow->getAddress();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getLabel() const {return m_label;}
const char* XCASH_SubaddressAccountRow_getLabel(void* subaddressAccountRow_ptr) {
    XCash::SubaddressAccountRow *subaddressAccountRow = reinterpret_cast<XCash::SubaddressAccountRow*>(subaddressAccountRow_ptr);
    std::string str = subaddressAccountRow->getLabel();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getBalance() const {return m_balance;}
const char* XCASH_SubaddressAccountRow_getBalance(void* subaddressAccountRow_ptr) {
    XCash::SubaddressAccountRow *subaddressAccountRow = reinterpret_cast<XCash::SubaddressAccountRow*>(subaddressAccountRow_ptr);
    std::string str = subaddressAccountRow->getBalance();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::string getUnlockedBalance() const {return m_unlockedBalance;}
const char* XCASH_SubaddressAccountRow_getUnlockedBalance(void* subaddressAccountRow_ptr) {
    XCash::SubaddressAccountRow *subaddressAccountRow = reinterpret_cast<XCash::SubaddressAccountRow*>(subaddressAccountRow_ptr);
    std::string str = subaddressAccountRow->getUnlockedBalance();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     std::size_t getRowId() const {return m_rowId;}
size_t XCASH_SubaddressAccountRow_getRowId(void* subaddressAccountRow_ptr) {
    XCash::SubaddressAccountRow *subaddressAccountRow = reinterpret_cast<XCash::SubaddressAccountRow*>(subaddressAccountRow_ptr);
    return subaddressAccountRow->getRowId();
}

// struct SubaddressAccount
// {
//     virtual ~SubaddressAccount() = 0;
//     virtual std::vector<SubaddressAccountRow*> getAll() const = 0;
int XCASH_SubaddressAccount_getAll_size(void* subaddressAccount_ptr) {
    XCash::SubaddressAccount *subaddress = reinterpret_cast<XCash::SubaddressAccount*>(subaddressAccount_ptr);
    return subaddress->getAll().size();
}
void* XCASH_SubaddressAccount_getAll_byIndex(void* subaddressAccount_ptr, int index) {
    XCash::SubaddressAccount *subaddress = reinterpret_cast<XCash::SubaddressAccount*>(subaddressAccount_ptr);
    return subaddress->getAll()[index];
}
//     virtual void addRow(const std::string &label) = 0;
void XCASH_SubaddressAccount_addRow(void* subaddressAccount_ptr, const char* label) {
    XCash::SubaddressAccount *subaddress = reinterpret_cast<XCash::SubaddressAccount*>(subaddressAccount_ptr);
    return subaddress->addRow(std::string(label));
}
//     virtual void setLabel(uint32_t accountIndex, const std::string &label) = 0;
void XCASH_SubaddressAccount_setLabel(void* subaddressAccount_ptr, uint32_t accountIndex, const char* label) {
    XCash::SubaddressAccount *subaddress = reinterpret_cast<XCash::SubaddressAccount*>(subaddressAccount_ptr);
    return subaddress->setLabel(accountIndex, std::string(label));
}
//     virtual void refresh() = 0;
void XCASH_SubaddressAccount_refresh(void* subaddressAccount_ptr) {
    XCash::SubaddressAccount *subaddress = reinterpret_cast<XCash::SubaddressAccount*>(subaddressAccount_ptr);
    return subaddress->refresh();
}

// MultisigState

//     bool isMultisig;
bool XCASH_MultisigState_isMultisig(void* multisigState_ptr) {
    XCash::MultisigState *multisigState = reinterpret_cast<XCash::MultisigState*>(multisigState_ptr);
    return multisigState->isMultisig;
}
//     bool isReady;
bool XCASH_MultisigState_isReady(void* multisigState_ptr) {
    XCash::MultisigState *multisigState = reinterpret_cast<XCash::MultisigState*>(multisigState_ptr);
    return multisigState->isReady;
}
//     uint32_t threshold;
uint32_t XCASH_MultisigState_threshold(void* multisigState_ptr) {
    XCash::MultisigState *multisigState = reinterpret_cast<XCash::MultisigState*>(multisigState_ptr);
    return multisigState->threshold;
}
//     uint32_t total;
uint32_t XCASH_MultisigState_total(void* multisigState_ptr) {
    XCash::MultisigState *multisigState = reinterpret_cast<XCash::MultisigState*>(multisigState_ptr);
    return multisigState->total;
}

// DeviceProgress


//     virtual double progress() const { return m_progress; }
bool XCASH_DeviceProgress_progress(void* deviceProgress_ptr) {
    throw std::runtime_error("XCASH_DeviceProgress_progress Not implemented");
    // XCash::DeviceProgress *deviceProgress = reinterpret_cast<XCash::DeviceProgress*>(deviceProgress_ptr);
    // return deviceProgress->progress();
}
//     virtual bool indeterminate() const { return m_indeterminate; }
bool XCASH_DeviceProgress_indeterminate(void* deviceProgress_ptr) {
    throw std::runtime_error("XCASH_DeviceProgress_indeterminate Not implemented");
    // XCash::DeviceProgress *deviceProgress = reinterpret_cast<XCash::DeviceProgress*>(deviceProgress_ptr);
    // return deviceProgress->indeterminate();
}

// Wallet

const char* XCASH_Wallet_seed(void* wallet_ptr, const char* seed_offset) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->seed();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_getSeedLanguage(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->getSeedLanguage();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

void XCASH_Wallet_setSeedLanguage(void* wallet_ptr, const char* arg) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setSeedLanguage(std::string(arg));
}

int XCASH_Wallet_status(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->status();
}

const char* XCASH_Wallet_errorString(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->errorString();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}


bool XCASH_Wallet_setPassword(void* wallet_ptr, const char* password) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setPassword(std::string(password));
}

const char* XCASH_Wallet_getPassword(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getPassword Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // std::string str = wallet->getPassword();
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}

bool XCASH_Wallet_setDevicePin(void* wallet_ptr, const char* pin) {
    throw std::runtime_error("XCASH_Wallet_setDevicePin Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->setDevicePin(std::string(pin));
}

bool XCASH_Wallet_setDevicePassphrase(void* wallet_ptr, const char* passphrase) {
    throw std::runtime_error("XCASH_Wallet_setDevicePassphrase Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->setDevicePassphrase(std::string(passphrase));
}

const char* XCASH_Wallet_address(void* wallet_ptr, uint64_t accountIndex, uint64_t addressIndex) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->address(accountIndex, addressIndex);
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_path(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->path();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
int XCASH_Wallet_nettype(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->nettype();
}
uint8_t XCASH_Wallet_useForkRules(void* wallet_ptr, uint8_t version, int64_t early_blocks) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->useForkRules(version, early_blocks);
}
const char* XCASH_Wallet_integratedAddress(void* wallet_ptr, const char* payment_id) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->integratedAddress(std::string(payment_id));
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_secretViewKey(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->secretViewKey();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_publicViewKey(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->publicViewKey();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_secretSpendKey(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->secretSpendKey();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_publicSpendKey(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->publicSpendKey();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
const char* XCASH_Wallet_publicMultisigSignerKey(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->publicMultisigSignerKey();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

void XCASH_Wallet_stop(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_stop Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // wallet->stop();
}

bool XCASH_Wallet_store(void* wallet_ptr, const char* path) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->store(std::string(path));
}
const char* XCASH_Wallet_filename(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->filename();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
const char* XCASH_Wallet_keysFilename(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->keysFilename();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

//     virtual bool init(const std::string &daemon_address, uint64_t upper_transaction_size_limit = 0, const std::string &daemon_username = "", const std::string &daemon_password = "", bool use_ssl = false, bool lightWallet = false, const std::string &proxy_address = "") = 0;
bool XCASH_Wallet_init(void* wallet_ptr, const char* daemon_address, uint64_t upper_transaction_size_limit, const char* daemon_username, const char* daemon_password, bool use_ssl, bool lightWallet, const char* proxy_address) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->init(std::string(daemon_address), upper_transaction_size_limit, std::string(daemon_username), std::string(daemon_password), use_ssl, lightWallet);
}
bool XCASH_Wallet_createWatchOnly(void* wallet_ptr, const char* path, const char* password, const char* language) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->createWatchOnly(std::string(path), std::string(password), std::string(language));
}

void XCASH_Wallet_setRefreshFromBlockHeight(void* wallet_ptr, uint64_t refresh_from_block_height) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setRefreshFromBlockHeight(refresh_from_block_height);
}

uint64_t XCASH_Wallet_getRefreshFromBlockHeight(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->getRefreshFromBlockHeight();
}

void XCASH_Wallet_setRecoveringFromSeed(void* wallet_ptr, bool recoveringFromSeed) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setRecoveringFromSeed(recoveringFromSeed);
}
void XCASH_Wallet_setRecoveringFromDevice(void* wallet_ptr, bool recoveringFromDevice) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setRecoveringFromDevice(recoveringFromDevice);
}
void XCASH_Wallet_setSubaddressLookahead(void* wallet_ptr, uint32_t major, uint32_t minor) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setSubaddressLookahead(major, minor);
}

bool XCASH_Wallet_connectToDaemon(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->connectToDaemon();
}
int XCASH_Wallet_connected(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->connected();
}
void XCASH_Wallet_setTrustedDaemon(void* wallet_ptr, bool arg) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setTrustedDaemon(arg);
}
bool XCASH_Wallet_trustedDaemon(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->trustedDaemon();
}
bool XCASH_Wallet_setProxy(void* wallet_ptr, const char* address) {
    throw std::runtime_error("XCASH_Wallet_setProxy not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->setProxy(std::string(address));
}

uint64_t XCASH_Wallet_balance(void* wallet_ptr, uint32_t accountIndex) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->balance(accountIndex);
}

uint64_t XCASH_Wallet_unlockedBalance(void* wallet_ptr, uint32_t accountIndex) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->unlockedBalance(accountIndex);
}

uint64_t XCASH_Wallet_viewOnlyBalance(void* wallet_ptr, uint32_t accountIndex) {
    throw std::runtime_error("XCASH_Wallet_viewOnlyBalance not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->viewOnlyBalance(accountIndex);
}

// TODO
bool XCASH_Wallet_watchOnly(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->watchOnly();
}
bool XCASH_Wallet_isDeterministic(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_isDeterministic not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->isDeterministic();
}
uint64_t XCASH_Wallet_blockChainHeight(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->blockChainHeight();
}
uint64_t XCASH_Wallet_approximateBlockChainHeight(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->approximateBlockChainHeight();
}
uint64_t XCASH_Wallet_estimateBlockChainHeight(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_estimateBlockChainHeight not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->estimateBlockChainHeight();
}
uint64_t XCASH_Wallet_daemonBlockChainHeight(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->daemonBlockChainHeight();
}

uint64_t daemonBlockChainHeight_cached = 0;

uint64_t XCASH_Wallet_daemonBlockChainHeight_cached(void* wallet_ptr) {
    return daemonBlockChainHeight_cached;
}

void XCASH_Wallet_daemonBlockChainHeight_runThread(void* wallet_ptr, int seconds) {
    std::cout << "DEPRECATED: this was used as an experiment, and will be removed in newer release. use ${COIN}_cw_* listener functions instead." << std::endl;
    while (true) {
        XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
        daemonBlockChainHeight_cached = wallet->daemonBlockChainHeight();
        sleep(seconds);
        std::cout << "XCASH: TICK: XCASH_Wallet_daemonBlockChainHeight_runThread(" << seconds << "): " << daemonBlockChainHeight_cached << std::endl;
    }
}

uint64_t XCASH_Wallet_daemonBlockChainTargetHeight(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->daemonBlockChainTargetHeight();
}
bool XCASH_Wallet_synchronized(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->synchronized();
}

const char* XCASH_Wallet_displayAmount(uint64_t amount) {
    std::string str = XCash::Wallet::displayAmount(amount);
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

//     static uint64_t amountFromString(const std::string &amount);
uint64_t XCASH_Wallet_amountFromString(const char* amount) {
    return XCash::Wallet::amountFromString(amount);
}
//     static uint64_t amountFromDouble(double amount);
uint64_t XCASH_Wallet_amountFromDouble(double amount) {
    return XCash::Wallet::amountFromDouble(amount);
}
//     static std::string genPaymentId();
const char* XCASH_Wallet_genPaymentId() {
    std::string str = XCash::Wallet::genPaymentId();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
//     static bool paymentIdValid(const std::string &paiment_id);
bool XCASH_Wallet_paymentIdValid(const char* paiment_id) {
    return XCash::Wallet::paymentIdValid(std::string(paiment_id));
}
bool XCASH_Wallet_addressValid(const char* str, int nettype) {
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return XCash::Wallet::addressValid(std::string(str), nettype);
}

bool XCASH_Wallet_keyValid(const char* secret_key_string, const char* address_string, bool isViewKey, int nettype) {
    std::string error;
    return XCash::Wallet::keyValid(std::string(secret_key_string), std::string(address_string), isViewKey, nettype, error);
}
const char* XCASH_Wallet_keyValid_error(const char* secret_key_string, const char* address_string, bool isViewKey, int nettype)  {
    std::string str;
    XCash::Wallet::keyValid(std::string(secret_key_string), std::string(address_string), isViewKey, nettype, str);
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
const char* XCASH_Wallet_paymentIdFromAddress(const char* strarg, int nettype) {
    std::string str = XCash::Wallet::paymentIdFromAddress(std::string(strarg), nettype);
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}
uint64_t XCASH_Wallet_maximumAllowedAmount() {
    return XCash::Wallet::maximumAllowedAmount();
}

void XCASH_Wallet_init3(void* wallet_ptr, const char* argv0, const char* default_log_base_name, const char* log_path, bool console) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->init(argv0, default_log_base_name, log_path, console);
}
const char* XCASH_Wallet_getPolyseed(void* wallet_ptr, const char* passphrase) {
    throw std::runtime_error("XCASH_Wallet_getPolyseed Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // std::string seed = "";
    // std::string _passphrase = std::string(passphrase);
    // wallet->getPolyseed(seed, _passphrase);
    // std::string str = seed;
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}
//     static bool createPolyseed(std::string &seed_words, std::string &err, const std::string &language = "English");
const char* XCASH_Wallet_createPolyseed(const char* language) {
    throw std::runtime_error("XCASH_Wallet_createPolyseed Not implemented");
    // std::string seed_words = "";
    // std::string err;
    // XCash::Wallet::createPolyseed(seed_words, err, std::string(language));
    // std::cout << "XCASH_Wallet_createPolyseed(language: " << language << "):" << std::endl;
    // std::cout << "           err: "  << err << std::endl;
    // std::string str = seed_words;
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}

void XCASH_Wallet_startRefresh(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->startRefresh();
}
void XCASH_Wallet_pauseRefresh(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->pauseRefresh();
}
bool XCASH_Wallet_refresh(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->refresh();
}
void XCASH_Wallet_refreshAsync(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->refreshAsync();
}
bool XCASH_Wallet_rescanBlockchain(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_rescanBlockchain Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->rescanBlockchain();
}
void XCASH_Wallet_rescanBlockchainAsync(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_rescanBlockchainAsync Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->rescanBlockchainAsync();
}
void XCASH_Wallet_setAutoRefreshInterval(void* wallet_ptr, int millis) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setAutoRefreshInterval(millis);
}
int XCASH_Wallet_autoRefreshInterval(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->autoRefreshInterval();
}
void XCASH_Wallet_addSubaddressAccount(void* wallet_ptr, const char* label) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->addSubaddressAccount(std::string(label));
}
size_t XCASH_Wallet_numSubaddressAccounts(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->numSubaddressAccounts();
}
size_t XCASH_Wallet_numSubaddresses(void* wallet_ptr, uint32_t accountIndex) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->numSubaddresses(accountIndex);
}
void XCASH_Wallet_addSubaddress(void* wallet_ptr, uint32_t accountIndex, const char* label) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->addSubaddress(accountIndex, std::string(label));
}
const char* XCASH_Wallet_getSubaddressLabel(void* wallet_ptr, uint32_t accountIndex, uint32_t addressIndex) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->getSubaddressLabel(accountIndex, addressIndex);
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

void XCASH_Wallet_setSubaddressLabel(void* wallet_ptr, uint32_t accountIndex, uint32_t addressIndex, const char* label) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setSubaddressLabel(accountIndex, addressIndex, std::string(label));
}

void* XCASH_Wallet_multisig(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    XCash::MultisigState *mstate_ptr = new XCash::MultisigState(wallet->multisig());
    return reinterpret_cast<void*>(mstate_ptr);
}

const char* XCASH_Wallet_getMultisigInfo(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->getMultisigInfo();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_makeMultisig(void* wallet_ptr, const char* info, const char* info_separator, uint32_t threshold) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->makeMultisig(splitStringVector(std::string(info), std::string(info_separator)), threshold);
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_exchangeMultisigKeys(void* wallet_ptr, const char* info, const char* info_separator, bool force_update_use_with_caution) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->exchangeMultisigKeys(splitStringVector(std::string(info), std::string(info_separator)));
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_exportMultisigImages(void* wallet_ptr, const char* separator) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str; 
    wallet->exportMultisigImages(str);
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

size_t XCASH_Wallet_importMultisigImages(void* wallet_ptr, const char* info, const char* info_separator) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->importMultisigImages(splitStringVector(std::string(info), std::string(info_separator)));
}

size_t XCASH_Wallet_hasMultisigPartialKeyImages(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->hasMultisigPartialKeyImages();
}

void* XCASH_Wallet_restoreMultisigTransaction(void* wallet_ptr, const char* signData) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return reinterpret_cast<void*>(wallet->restoreMultisigTransaction(std::string(signData)));
}


XCash::PendingTransaction::Priority PendingTransaction_Priority_fromInt(int value) {
    switch(value) {
        case 0: return XCash::PendingTransaction::Priority::Priority_Default;
        case 1: return XCash::PendingTransaction::Priority::Priority_Low;
        case 2: return XCash::PendingTransaction::Priority::Priority_Medium;
        case 3: return XCash::PendingTransaction::Priority::Priority_High;
        default: return XCash::PendingTransaction::Priority::Priority_Default;
    }
}

void* XCASH_Wallet_createTransactionMultDest(void* wallet_ptr, const char* dst_addr_list, const char* dst_addr_list_separator, const char* payment_id,
                                                bool amount_sweep_all, const char* amount_list, const char* amount_list_separator, uint32_t mixin_count,
                                                int pendingTransactionPriority,
                                                uint32_t subaddr_account,
                                                const char* preferredInputs, const char* preferredInputs_separator) {
    throw std::runtime_error("XCASH_Wallet_createTransactionMultDest Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // std::vector<std::string> dst_addr = splitStringVector(std::string(dst_addr_list), std::string(dst_addr_list_separator));

    // XCash::optional<std::vector<uint64_t>> optAmount;
    // if (!amount_sweep_all) {
    //     optAmount = splitStringUint(std::string(amount_list), std::string(amount_list_separator));;
    // }
    // std::set<uint32_t> subaddr_indices = {};
    // std::set<std::string> preferred_inputs = splitString(std::string(preferredInputs), std::string(preferredInputs_separator));

    // return wallet->createTransactionMultDest(
    //     dst_addr, std::string(payment_id),
    //     optAmount, mixin_count,
    //     PendingTransaction_Priority_fromInt(pendingTransactionPriority),
    //     subaddr_account,
    //     subaddr_indices,
    //     preferred_inputs
    // );
}

void* XCASH_Wallet_createTransaction(void* wallet_ptr, const char* dst_addr, const char* payment_id,
                                                    uint64_t amount, uint32_t mixin_count,
                                                    int pendingTransactionPriority,
                                                    uint32_t subaddr_account,
                                                    const char* preferredInputs, const char* separator) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    XCash::optional<uint64_t> optAmount;
    if (amount != 0) {
        optAmount = amount;
    }
    std::set<uint32_t> subaddr_indices = {};
    std::set<std::string> preferred_inputs = splitString(std::string(preferredInputs), std::string(separator));
    return wallet->createTransaction(std::string(dst_addr), std::string(payment_id),
                                        optAmount, mixin_count,
                                        PendingTransaction_Priority_fromInt(pendingTransactionPriority),
                                        subaddr_account, subaddr_indices);
}

void* XCASH_Wallet_loadUnsignedTx(void* wallet_ptr, const char* fileName) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->loadUnsignedTx(std::string(fileName));
}

void* XCASH_Wallet_loadUnsignedTxUR(void* wallet_ptr, const char* input) {
    throw std::runtime_error("XCASH_Wallet_loadUnsignedTxUR Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->loadUnsignedTxUR(std::string(input));
}
bool XCASH_Wallet_submitTransaction(void* wallet_ptr, const char* fileName) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->submitTransaction(std::string(fileName));
}
bool XCASH_Wallet_submitTransactionUR(void* wallet_ptr, const char* input) {
    throw std::runtime_error("XCASH_Wallet_submitTransactionUR Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->submitTransactionUR(std::string(input));
}
bool XCASH_Wallet_hasUnknownKeyImages(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_hasUnknownKeyImages Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->hasUnknownKeyImages();
}
bool XCASH_Wallet_exportKeyImages(void* wallet_ptr, const char* filename, bool all) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->exportKeyImages(std::string(filename));
}

const char* XCASH_Wallet_exportKeyImagesUR(void* wallet_ptr, size_t max_fragment_length, bool all) {
    throw std::runtime_error("XCASH_Wallet_exportKeyImagesUR Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // std::string str = wallet->exportKeyImagesUR(max_fragment_length, all);
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}
bool XCASH_Wallet_importKeyImages(void* wallet_ptr, const char* filename) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->importKeyImages(std::string(filename));
}
bool XCASH_Wallet_importKeyImagesUR(void* wallet_ptr, const char* input) {
    throw std::runtime_error("XCASH_Wallet_importKeyImagesUR Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->importKeyImagesUR(std::string(input));
}
bool XCASH_Wallet_exportOutputs(void* wallet_ptr, const char* filename, bool all) {
    throw std::runtime_error("XCASH_Wallet_exportOutputs Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->exportOutputs(std::string(filename), all);
}
const char* XCASH_Wallet_exportOutputsUR(void* wallet_ptr, size_t max_fragment_length, bool all) {
    throw std::runtime_error("XCASH_Wallet_exportOutputsUR Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // std::string str = wallet->exportOutputsUR(max_fragment_length, all);
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}
bool XCASH_Wallet_importOutputs(void* wallet_ptr, const char* filename) {
    throw std::runtime_error("XCASH_Wallet_importOutputs Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->importOutputs(std::string(filename));
}
bool XCASH_Wallet_importOutputsUR(void* wallet_ptr, const char* input) {
    throw std::runtime_error("XCASH_Wallet_importOutputsUR Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->importOutputsUR(std::string(input));
}
//     virtual bool setupBackgroundSync(const BackgroundSyncType background_sync_type, const std::string &wallet_password, const optional<std::string> &background_cache_password) = 0;
bool XCASH_Wallet_setupBackgroundSync(void* wallet_ptr, int background_sync_type, const char* wallet_password, const char* background_cache_password) {
    throw std::runtime_error("XCASH_Wallet_setupBackgroundSync Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->setupBackgroundSync(XCash::Wallet::BackgroundSyncType::BackgroundSync_CustomPassword, std::string(wallet_password), std::string(background_cache_password));
}
//     virtual BackgroundSyncType getBackgroundSyncType() const = 0;
int XCASH_Wallet_getBackgroundSyncType(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getBackgroundSyncType Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getBackgroundSyncType();
}
//     virtual bool startBackgroundSync() = 0;
bool XCASH_Wallet_startBackgroundSync(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_startBackgroundSync Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->startBackgroundSync();
}
//     virtual bool stopBackgroundSync(const std::string &wallet_password) = 0;
bool XCASH_Wallet_stopBackgroundSync(void* wallet_ptr, const char* wallet_password) {
    throw std::runtime_error("XCASH_Wallet_stopBackgroundSync Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->stopBackgroundSync(std::string(wallet_password));
}
//     virtual bool isBackgroundSyncing() const = 0;
bool XCASH_Wallet_isBackgroundSyncing(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_isBackgroundSyncing Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->hasUnknownKeyImages();
}
//     virtual bool isBackgroundWallet() const = 0;
bool XCASH_Wallet_isBackgroundWallet(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_isBackgroundWallet Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->isBackgroundWallet();
}
void* XCASH_Wallet_history(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->history();
}
void* XCASH_Wallet_addressBook(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->addressBook();
}
//     virtual Coins * coins() = 0;
void* XCASH_Wallet_coins(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_coins Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->coins();
}
//     virtual Subaddress * subaddress() = 0;
void* XCASH_Wallet_subaddress(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->subaddress();
}
//     virtual SubaddressAccount * subaddressAccount() = 0;
void* XCASH_Wallet_subaddressAccount(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->subaddressAccount();
}
//     virtual uint32_t defaultMixin() const = 0;
uint32_t XCASH_Wallet_defaultMixin(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->defaultMixin();
}
//     virtual void setDefaultMixin(uint32_t arg) = 0;
void XCASH_Wallet_setDefaultMixin(void* wallet_ptr, uint32_t arg) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setDefaultMixin(arg);
}
//     virtual bool setCacheAttribute(const std::string &key, const std::string &val) = 0;
bool XCASH_Wallet_setCacheAttribute(void* wallet_ptr, const char* key, const char* val) {
    throw std::runtime_error("XCASH_Wallet_setCacheAttribute Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->setCacheAttribute(std::string(key), std::string(val));
}
//     virtual std::string getCacheAttribute(const std::string &key) const = 0;
const char* XCASH_Wallet_getCacheAttribute(void* wallet_ptr, const char* key) {
    throw std::runtime_error("XCASH_Wallet_getCacheAttribute Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // std::string str = wallet->getCacheAttribute(std::string(key));
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}
//     virtual bool setUserNote(const std::string &txid, const std::string &note) = 0;
bool XCASH_Wallet_setUserNote(void* wallet_ptr, const char* txid, const char* note) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->setUserNote(std::string(txid), std::string(note));
}
//     virtual std::string getUserNote(const std::string &txid) const = 0;
const char* XCASH_Wallet_getUserNote(void* wallet_ptr, const char* txid) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->getUserNote(std::string(txid));
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_getTxKey(void* wallet_ptr, const char* txid) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->getTxKey(std::string(txid));
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

const char* XCASH_Wallet_signMessage(void* wallet_ptr, const char* message, const char* address) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    std::string str = wallet->signMessage(std::string(message));
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

bool XCASH_Wallet_verifySignedMessage(void* wallet_ptr, const char* message, const char* address, const char* signature) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    bool v = wallet->verifySignedMessage(std::string(message), std::string(address), std::string(signature));
    return v;
}

bool XCASH_Wallet_rescanSpent(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->rescanSpent();
}

void XCASH_Wallet_setOffline(void* wallet_ptr, bool offline) {
    throw std::runtime_error("XCASH_Wallet_setOffline not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->setOffline(offline);
}
//     virtual bool isOffline() const = 0;
bool XCASH_Wallet_isOffline(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_isOffline not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->isOffline();
}

void XCASH_Wallet_segregatePreForkOutputs(void* wallet_ptr, bool segregate) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->segregatePreForkOutputs(segregate);
}
//     virtual void segregationHeight(uint64_t height) = 0;
void XCASH_Wallet_segregationHeight(void* wallet_ptr, uint64_t height) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->segregationHeight(height);
}
//     virtual void keyReuseMitigation2(bool mitigation) = 0;
void XCASH_Wallet_keyReuseMitigation2(void* wallet_ptr, bool mitigation) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->keyReuseMitigation2(mitigation);
}
//     virtual bool lightWalletLogin(bool &isNewWallet) const = 0;
//     virtual bool lightWalletImportWalletRequest(std::string &payment_id, uint64_t &fee, bool &new_request, bool &request_fulfilled, std::string &payment_address, std::string &status) = 0;
//     virtual bool lockKeysFile() = 0;
bool XCASH_Wallet_lockKeysFile(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->lockKeysFile();
}
//     virtual bool unlockKeysFile() = 0;
bool XCASH_Wallet_unlockKeysFile(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->unlockKeysFile();
}
//     virtual bool isKeysFileLocked() = 0;
bool XCASH_Wallet_isKeysFileLocked(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->isKeysFileLocked();
}
//     virtual Device getDeviceType() const = 0;
int XCASH_Wallet_getDeviceType(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wallet->getDeviceType();
}
//     virtual uint64_t coldKeyImageSync(uint64_t &spent, uint64_t &unspent) = 0;
uint64_t XCASH_Wallet_coldKeyImageSync(void* wallet_ptr, uint64_t spent, uint64_t unspent) {
    throw std::runtime_error("XCASH_Wallet_coldKeyImageSync Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->coldKeyImageSync(spent, unspent);
}
//     virtual void deviceShowAddress(uint32_t accountIndex, uint32_t addressIndex, const std::string &paymentId) = 0;
const char* XCASH_Wallet_deviceShowAddress(void* wallet_ptr, uint32_t accountIndex, uint32_t addressIndex) {
    throw std::runtime_error("XCASH_Wallet_deviceShowAddress Not implemented"); 
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // std::string str = "";
    // wallet->deviceShowAddress(accountIndex, addressIndex, str);
    // const std::string::size_type size = str.size();
    // char *buffer = new char[size + 1];   //we need extra char for NUL
    // memcpy(buffer, str.c_str(), size + 1);
    // return buffer;
}
//     virtual bool reconnectDevice() = 0;
bool XCASH_Wallet_reconnectDevice(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_reconnectDevice Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->reconnectDevice();
};

uint64_t XCASH_Wallet_getBytesReceived(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getBytesReceived Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getBytesReceived();
}
uint64_t XCASH_Wallet_getBytesSent(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getBytesSent Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getBytesSent();
}

bool XCASH_Wallet_getStateIsConnected(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getStateIsConnected Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getStateIsConnected();
}

unsigned char* XCASH_Wallet_getSendToDevice(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getSendToDevice Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getSendToDevice();
}

size_t XCASH_Wallet_getSendToDeviceLength(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getSendToDeviceLength Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getSendToDeviceLength();
}

unsigned char* XCASH_Wallet_getReceivedFromDevice(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getReceivedFromDevice Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getReceivedFromDevice();
}

size_t XCASH_Wallet_getReceivedFromDeviceLength(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getReceivedFromDeviceLength Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getReceivedFromDeviceLength();
}

bool XCASH_Wallet_getWaitsForDeviceSend(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getWaitsForDeviceSend Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getWaitsForDeviceSend();
}

bool XCASH_Wallet_getWaitsForDeviceReceive(void* wallet_ptr) {
    throw std::runtime_error("XCASH_Wallet_getWaitsForDeviceReceive Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->getWaitsForDeviceReceive();
}

void XCASH_Wallet_setDeviceReceivedData(void* wallet_ptr, unsigned char* data, size_t len) {
    throw std::runtime_error("XCASH_Wallet_setDeviceReceivedData Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->setDeviceReceivedData(data, len);
}

void XCASH_Wallet_setDeviceSendData(void* wallet_ptr, unsigned char* data, size_t len) {
    throw std::runtime_error("XCASH_Wallet_setDeviceSendData Not implemented");
    // XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    // return wallet->setDeviceSendData(data, len);
}

void* XCASH_WalletManager_createWallet(void* wm_ptr, const char* path, const char* password, const char* language, int networkType) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    XCash::Wallet *wallet = wm->createWallet(
                    std::string(path),
                    std::string(password),
                    std::string(language),
                    static_cast<XCash::NetworkType>(networkType));
    return reinterpret_cast<void*>(wallet);
}

void* XCASH_WalletManager_openWallet(void* wm_ptr, const char* path, const char* password, int networkType) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    XCash::Wallet *wallet = wm->openWallet(
                    std::string(path),
                    std::string(password),
                    static_cast<XCash::NetworkType>(networkType));
    return reinterpret_cast<void*>(wallet);
}
void* XCASH_WalletManager_recoveryWallet(void* wm_ptr, const char* path, const char* password, const char* mnemonic, int networkType, uint64_t restoreHeight, uint64_t kdfRounds, const char* seedOffset) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    // (const std::string &path, const std::string &password, const std::string &mnemonic,
    //                                     NetworkType nettype = MAINNET, uint64_t restoreHeight = 0, uint64_t kdf_rounds = 1,
    //                                     const std::string &seed_offset = {})
    XCash::Wallet *wallet = wm->recoveryWallet(
                    std::string(path),
                    std::string(mnemonic),
                    static_cast<XCash::NetworkType>(networkType),
                    restoreHeight);
    return reinterpret_cast<void*>(wallet);
}
//     virtual Wallet * createWalletFromKeys(const std::string &path,
//                                                     const std::string &password,
//                                                     const std::string &language,
//                                                     NetworkType nettype,
//                                                     uint64_t restoreHeight,
//                                                     const std::string &addressString,
//                                                     const std::string &viewKeyString,
//                                                     const std::string &spendKeyString = "",
//                                                     uint64_t kdf_rounds = 1) = 0;
void* XCASH_WalletManager_createWalletFromKeys(void* wm_ptr, const char* path, const char* password, const char* language, int nettype, uint64_t restoreHeight, const char* addressString, const char* viewKeyString, const char* spendKeyString, uint64_t kdf_rounds) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    XCash::Wallet *wallet = wm->createWalletFromKeys(
                    std::string(path),
                    std::string(password),
                    std::string(language),
                    static_cast<XCash::NetworkType>(nettype),
                    restoreHeight,
                    std::string(addressString),
                    std::string(viewKeyString),
                    std::string(spendKeyString));
    return reinterpret_cast<void*>(wallet);
}

void* XCASH_WalletManager_createWalletFromDevice(void* wm_ptr, const char* path, const char* password, int nettype, const char* deviceName, uint64_t restoreHeight, const char* subaddressLookahead, const char* viewKeyString, const char* spendKeyString, uint64_t kdf_rounds) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    XCash::Wallet *wallet = wm->createWalletFromDevice(std::string(path),
        std::string(password),
        static_cast<XCash::NetworkType>(nettype),
        std::string(deviceName),
        restoreHeight,
        std::string(subaddressLookahead),
        kdf_rounds);
    return reinterpret_cast<void*>(wallet);
}

void* XCASH_WalletManager_createDeterministicWalletFromSpendKey(void* wm_ptr, const char* path, const char* password,
                                                const char* language, int nettype, uint64_t restoreHeight,
                                                const char* spendKeyString, uint64_t kdf_rounds) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    throw std::runtime_error("XCASH_WalletManager_createDeterministicWalletFromSpendKey Not implemented");
    // XCash::Wallet *wallet = wm->createDeterministicWalletFromSpendKey(
    //     std::string(path),
    //     std::string(password),
    //     std::string(language),
    //     static_cast<XCash::NetworkType>(nettype),
    //     restoreHeight,
    //     std::string(spendKeyString),
    //     kdf_rounds
    // );
    // return reinterpret_cast<void*>(wallet);
}

void* XCASH_WalletManager_createWalletFromPolyseed(void* wm_ptr, const char* path, const char* password,
                                                int nettype, const char* mnemonic, const char* passphrase,
                                                bool newWallet, uint64_t restore_height, uint64_t kdf_rounds) {
    throw std::runtime_error("XCASH_WalletManager_createWalletFromPolyseed Not implemented");

    // XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    // return wm->createWalletFromPolyseed(std::string(path),
    //                                           std::string(password),
    //                                           static_cast<XCash::NetworkType>(nettype),
    //                                           std::string(mnemonic),
    //                                           std::string(passphrase),
    //                                           newWallet,
    //                                           restore_height,
    //                                           kdf_rounds);
}


bool XCASH_WalletManager_closeWallet(void* wm_ptr, void* wallet_ptr, bool store) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return wm->closeWallet(wallet, store);
}

bool XCASH_WalletManager_walletExists(void* wm_ptr, const char* path) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    return wm->walletExists(std::string(path));
}

//     virtual bool verifyWalletPassword(const std::string &keys_file_name, const std::string &password, bool no_spend_key, uint64_t kdf_rounds = 1) const = 0;
bool XCASH_WalletManager_verifyWalletPassword(void* wm_ptr, const char* keys_file_name, const char* password, bool no_spend_key, uint64_t kdf_rounds) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    return wm->verifyWalletPassword(std::string(keys_file_name), std::string(password), no_spend_key, kdf_rounds);
}

//     virtual bool queryWalletDevice(Wallet::Device& device_type, const std::string &keys_file_name, const std::string &password, uint64_t kdf_rounds = 1) const = 0;
int XCASH_WalletManager_queryWalletDevice(void* wm_ptr, const char* keys_file_name, const char* password, uint64_t kdf_rounds) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    XCash::Wallet::Device device_type;
    wm->queryWalletDevice(device_type, std::string(keys_file_name), std::string(password), kdf_rounds);
    return device_type;
}

//     virtual std::vector<std::string> findWallets(const std::string &path) = 0;
const char* XCASH_WalletManager_findWallets(void* wm_ptr, const char* path, const char* separator) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    return vectorToString(wm->findWallets(std::string(path)), std::string(separator));
}


const char* XCASH_WalletManager_errorString(void* wm_ptr) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    std::string str = wm->errorString();
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

void XCASH_WalletManager_setDaemonAddress(void* wm_ptr, const char* address) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    return wm->setDaemonAddress(std::string(address));
}

bool XCASH_WalletManager_setProxy(void* wm_ptr, const char* address) {
    throw std::runtime_error("XCASH_WalletManager_setProxy Not implemented");
    // XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    // return wm->setProxy(std::string(address));
}


//     virtual bool connected(uint32_t *version = NULL) = 0;
//     virtual uint64_t blockchainHeight() = 0;
uint64_t XCASH_WalletManager_blockchainHeight(void* wm_ptr) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    return wm->blockchainHeight();
}
//     virtual uint64_t blockchainTargetHeight() = 0;
uint64_t XCASH_WalletManager_blockchainTargetHeight(void* wm_ptr) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    return wm->blockchainTargetHeight();
}
//     virtual uint64_t networkDifficulty() = 0;
uint64_t XCASH_WalletManager_networkDifficulty(void* wm_ptr) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    return wm->networkDifficulty();
}
//     virtual double miningHashRate() = 0;
double XCASH_WalletManager_miningHashRate(void* wm_ptr) {
    return 0.0;
    // XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    // return wm->miningHashRate();
}
//     virtual uint64_t blockTarget() = 0;
uint64_t XCASH_WalletManager_blockTarget(void* wm_ptr) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    return wm->blockTarget();
}
//     virtual bool isMining() = 0;
bool XCASH_WalletManager_isMining(void* wm_ptr) {
    return false;
    // XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    // return wm->isMining();
}
//     virtual bool startMining(const std::string &address, uint32_t threads = 1, bool background_mining = false, bool ignore_battery = true) = 0;
bool XCASH_WalletManager_startMining(void* wm_ptr, const char* address, uint32_t threads, bool backgroundMining, bool ignoreBattery) {
    return false;
    throw std::runtime_error("XCASH_WalletManager_startMining Not implemented");
    // XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    // return wm->startMining(std::string(address), threads, backgroundMining, ignoreBattery);
}
//     virtual bool stopMining() = 0;
bool XCASH_WalletManager_stopMining(void* wm_ptr, const char* address) {
    return true;
    // XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    // return wm->stopMining();
}
//     virtual std::string resolveOpenAlias(const std::string &address, bool &dnssec_valid) const = 0;
const char* XCASH_WalletManager_resolveOpenAlias(void* wm_ptr, const char* address, bool dnssec_valid) {
    XCash::WalletManager *wm = reinterpret_cast<XCash::WalletManager*>(wm_ptr);
    std::string str = wm->resolveOpenAlias(std::string(address), dnssec_valid);
    const std::string::size_type size = str.size();
    char *buffer = new char[size + 1];   //we need extra char for NUL
    memcpy(buffer, str.c_str(), size + 1);
    return buffer;
}

// WalletManagerFactory

void* XCASH_WalletManagerFactory_getWalletManager() {
    XCash::WalletManager *wm = XCash::WalletManagerFactory::getWalletManager();
    return reinterpret_cast<void*>(wm);
}

void XCASH_WalletManagerFactory_setLogLevel(int level) {
    XCash::WalletManagerFactory::setLogLevel(level);
}

void XCASH_WalletManagerFactory_setLogCategories(const char* categories) {
    XCash::WalletManagerFactory::setLogCategories(std::string(categories));
}

// DEBUG functions

// As it turns out we need a bit more functions to make sure that the library is working.
// 0) void
// 1) bool
// 2) int
// 3) uint64_t
// 4) void*
// 5) const char* 

void XCASH_DEBUG_test0() {
    return;
}

bool XCASH_DEBUG_test1(bool x) {
    return x;
}

int XCASH_DEBUG_test2(int x) {
    return x;
}

uint64_t XCASH_DEBUG_test3(uint64_t x) {
    return x;
}

void* XCASH_DEBUG_test4(uint64_t x) {
    int *y = new int(x);
    return reinterpret_cast<void*>(y);
}

const char* XCASH_DEBUG_test5() {
    const char *text  = "This is a const char* text"; 
    return text;
}

const char* XCASH_DEBUG_test5_std() {
    std::string text ("This is a std::string text");
    const char *text2 = "This is a text"; 
    return text2;
}

bool XCASH_DEBUG_isPointerNull(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    return (wallet != NULL);
}

// cake wallet world
// TODO(mrcyjanek): https://api.dart.dev/stable/3.3.3/dart-ffi/Pointer/fromFunction.html
//                  callback to dart should be possible..? I mean why not? But I need to
//                  wait for other implementation (Go preferably) to see if this approach
//                  will work as expected.
struct XCASH_cw_WalletListener;
struct XCASH_cw_WalletListener : XCash::WalletListener
{
    uint64_t m_height;
    bool m_need_to_refresh;
    bool m_new_transaction;

    XCASH_cw_WalletListener()
    {
        m_height = 0;
        m_need_to_refresh = false;
        m_new_transaction = false;
    }

    void moneySpent(const std::string &txId, uint64_t amount)
    {
        m_new_transaction = true;
    }

    void moneyReceived(const std::string &txId, uint64_t amount)
    {
        m_new_transaction = true;
    }

    void unconfirmedMoneyReceived(const std::string &txId, uint64_t amount)
    {
        m_new_transaction = true;
    }

    void newBlock(uint64_t height)
    {
        m_height = height;
    }

    void updated()
    {
        m_new_transaction = true;
    }

    void refreshed()
    {
        m_need_to_refresh = true;
    }


    void cw_resetNeedToRefresh()
    {
        m_need_to_refresh = false;
    }

    bool cw_isNeedToRefresh()
    {
        return m_need_to_refresh;
    }

    bool cw_isNewTransactionExist()
    {
        return m_new_transaction;
    }

    void cw_resetIsNewTransactionExist()
    {
        m_new_transaction = false;
    }

    uint64_t cw_height()
    {
        return m_height;
    }
};

void* XCASH_cw_getWalletListener(void* wallet_ptr) {
    XCash::Wallet *wallet = reinterpret_cast<XCash::Wallet*>(wallet_ptr);
    XCASH_cw_WalletListener *listener = new XCASH_cw_WalletListener();
    wallet->setListener(listener);
    return reinterpret_cast<void*>(listener);
}

void XCASH_cw_WalletListener_resetNeedToRefresh(void* cw_walletListener_ptr) {
    XCASH_cw_WalletListener *listener = reinterpret_cast<XCASH_cw_WalletListener*>(cw_walletListener_ptr);
    listener->cw_resetNeedToRefresh();
}

bool XCASH_cw_WalletListener_isNeedToRefresh(void* cw_walletListener_ptr) {
    XCASH_cw_WalletListener *listener = reinterpret_cast<XCASH_cw_WalletListener*>(cw_walletListener_ptr);
    return listener->cw_isNeedToRefresh();
};

bool XCASH_cw_WalletListener_isNewTransactionExist(void* cw_walletListener_ptr) {
    XCASH_cw_WalletListener *listener = reinterpret_cast<XCASH_cw_WalletListener*>(cw_walletListener_ptr);
    return listener->cw_isNeedToRefresh();
};

void XCASH_cw_WalletListener_resetIsNewTransactionExist(void* cw_walletListener_ptr) {
    XCASH_cw_WalletListener *listener = reinterpret_cast<XCASH_cw_WalletListener*>(cw_walletListener_ptr);
    listener->cw_isNeedToRefresh();
};

uint64_t XCASH_cw_WalletListener_height(void* cw_walletListener_ptr) {
    XCASH_cw_WalletListener *listener = reinterpret_cast<XCASH_cw_WalletListener*>(cw_walletListener_ptr);
    return listener->cw_isNeedToRefresh();
};

const char* XCASH_checksum_wallet2_api_c_h() {
    return XCASH_wallet2_api_c_h_sha256;
}
const char* XCASH_checksum_wallet2_api_c_cpp() {
    return XCASH_wallet2_api_c_cpp_sha256;
}
const char* XCASH_checksum_wallet2_api_c_exp() {
    return XCASH_wallet2_api_c_exp_sha256;
}
// i hate windows

void XCASH_free(void* ptr) {
    free(ptr);
}

#ifdef __cplusplus
}
#endif
