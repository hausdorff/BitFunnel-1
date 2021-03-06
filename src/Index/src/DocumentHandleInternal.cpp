// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "DocumentHandleInternal.h"
#include "DocTableDescriptor.h"
#include "LoggerInterfaces/Logging.h"
#include "Shard.h"
#include "Slice.h"


namespace BitFunnel
{
    DocumentHandle Factories::CreateDocumentHandle(void * sliceBuffer, DocIndex index)
    {
        Slice* slice = *reinterpret_cast<Slice**>(
            static_cast<char *>(sliceBuffer) + Shard::GetSlicePtrOffset());
        return DocumentHandleInternal(slice, index);
    }


    //*************************************************************************
    //
    // DocumentHandle
    //
    //*************************************************************************
    DocumentHandle::DocumentHandle(Slice* slice, DocIndex index)
      : m_slice(slice),
        m_index(index)
    {
    }


    void* DocumentHandle::AllocateVariableSizeBlob(VariableSizeBlobId id, size_t byteSize)
    {
        return m_slice->GetDocTable().
            AllocateVariableSizeBlob(m_slice->GetSliceBuffer(),
                                     m_index,
                                     id,
                                     byteSize);
    }


    void* DocumentHandle::GetVariableSizeBlob(VariableSizeBlobId id) const
    {
        return m_slice->GetDocTable().
            GetVariableSizeBlob(m_slice->GetSliceBuffer(),
                                m_index,
                                id);
    }


    void* DocumentHandle::GetFixedSizeBlob(FixedSizeBlobId id) const
    {
        return m_slice->GetDocTable().
            GetFixedSizeBlob(m_slice->GetSliceBuffer(),
                             m_index,
                             id);
    }


    void DocumentHandle::AssertFact(FactHandle fact, bool value)
    {
        m_slice->GetShard().AssertFact(fact,
                                       value,
                                       m_index,
                                       m_slice->GetSliceBuffer());
    }


    void DocumentHandle::AddPosting(Term const & term)
    {
        m_slice->GetShard().AddPosting(term, m_index, m_slice->GetSliceBuffer());
    }


    void DocumentHandle::Expire()
    {
        const RowId documentActiveRow = m_slice->GetShard().GetDocumentActiveRowId();

        RowTableDescriptor const & rowTable = m_slice->GetRowTable(documentActiveRow.GetRank());
        rowTable.ClearBit(m_slice->GetSliceBuffer(), documentActiveRow.GetIndex(), m_index);

        const bool isSliceExpired = m_slice->ExpireDocument();
        if (isSliceExpired)
        {
            // All documents are expired in the Slice and the index is
            // abandoning its reference to this Slice. If this was the only
            // reference, then the Slice is scheduled for backup.
            Slice::DecrementRefCount(m_slice);
        }
    }


    DocId DocumentHandle::GetDocId() const
    {
        return m_slice->GetDocTable().GetDocId(m_slice->GetSliceBuffer(),
                                               m_index);
    }


    bool DocumentHandle::GetBit(RowId row) const
    {
        auto bit =
            m_slice->GetShard().GetRowTable(
                row.GetRank()).GetBit(
                    m_slice->GetSliceBuffer(),
                    row.GetIndex(),
                    m_index);

        return bit == 1ull;
    }


    //*************************************************************************
    //
    // DocumentHandleInternal
    //
    //*************************************************************************

    DocumentHandleInternal::DocumentHandleInternal()
        : DocumentHandle(nullptr, c_invalidDocIndex)
    {
    }


    DocumentHandleInternal::DocumentHandleInternal(Slice* slice, DocIndex index)
        : DocumentHandle(slice, index)
    {
    }


    DocumentHandleInternal::DocumentHandleInternal(Slice* slice,
                                                   DocIndex index,
                                                   DocId id)
        : DocumentHandle(slice, index)
    {
        m_slice->GetDocTable().SetDocId(GetSlice()->GetSliceBuffer(),
                                        GetIndex(),
                                        id);
    }


    DocumentHandleInternal::DocumentHandleInternal(DocumentHandle const & handle)
        : DocumentHandle(handle)
    {
    }


    Slice* DocumentHandleInternal::GetSlice() const
    {
        return m_slice;
    }


    DocIndex DocumentHandleInternal::GetIndex() const
    {
        return m_index;
    }


    void DocumentHandleInternal::Activate()
    {
        const RowId documentActiveRow =
            m_slice->GetShard().GetDocumentActiveRowId();
        RowTableDescriptor const & rowTable =
            m_slice->GetRowTable(documentActiveRow.GetRank());
        rowTable.SetBit(m_slice->GetSliceBuffer(),
                        documentActiveRow.GetIndex(),
                        m_index);

        m_slice->GetShard().TemporaryRecordDocument();
    }
}
