// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#include "ph.h"
#include "beIntervalTree.h"
#include "beSegmentsManager.h"
#include "beViewportArea.h"
#include "beTreeUtils.h"
#include "beUtils.h"
#include "beConfig.h"

namespace be
{

namespace
{

struct SHeapItem;
struct SHeapNode;
struct SHeapLeaf;

class KHeapItemVisitor
{
	protected:
		KHeapItemVisitor() = default;

	public:
		virtual ~KHeapItemVisitor() = default;

	public:
		virtual void visitNode( SHeapNode* node );
		virtual void visitLeaf( SHeapLeaf* leaf );
		virtual void visitDefault( SHeapItem* item );

};

// ----------------------------------------------------------------------------

struct SHeapItem
{
	protected:
		explicit SHeapItem( const SSectionPos* sectpos );

	public:
		virtual ~SHeapItem() = default;

	public:
		virtual SHeapItem* getLeftChild() = 0;
		virtual SHeapItem* getRightChild() = 0;
		virtual const SSectionPos* getMedian() = 0;

		virtual void accept( KHeapItemVisitor* visitor ) = 0;

	public:
		const SSectionPos* d_sectpos;
};

SHeapItem::SHeapItem( const SSectionPos* sectpos )
	: d_sectpos( sectpos )
{
}

// ----------------------------------------------------------------------------

struct SHeapNode : public SHeapItem
{
	public:
		SHeapNode(
			const SSectionPos* sectpos,
			const SSectionPos* median );
		~SHeapNode() override;

	public:
		SHeapItem* getLeftChild() override;
		SHeapItem* getRightChild() override;
		const SSectionPos* getMedian() override;

		void accept( KHeapItemVisitor* visitor ) override;

	public:
		const SSectionPos* d_median;
		SHeapItem* d_leftChild;
		SHeapItem* d_rightChild;

};

SHeapNode::SHeapNode(
	const SSectionPos* sectpos,
	const SSectionPos* median )
	: SHeapItem( sectpos )
	, d_median( median )
	, d_leftChild( nullptr )
	, d_rightChild( nullptr )
{
}

SHeapNode::~SHeapNode()
{
	delete d_leftChild;
	delete d_rightChild;
}

SHeapItem* SHeapNode::getLeftChild()
{
	return d_leftChild;
}

SHeapItem* SHeapNode::getRightChild()
{
	return d_rightChild;
}

const SSectionPos* SHeapNode::getMedian()
{
	return d_median;
}

void SHeapNode::accept( KHeapItemVisitor* visitor )
{
	visitor->visitNode( this );
}

// ----------------------------------------------------------------------------

struct SHeapLeaf : public SHeapItem
{
	public:
		explicit SHeapLeaf( const SSectionPos* sectpos );

	public:
		SHeapItem* getLeftChild() override;
		SHeapItem* getRightChild() override;
		const SSectionPos* getMedian() override;

		void accept( KHeapItemVisitor* visitor ) override;

};

SHeapLeaf::SHeapLeaf( const SSectionPos* sectpos )
	: SHeapItem( sectpos )
{
}

SHeapItem* SHeapLeaf::getLeftChild()
{
	return nullptr;
}

SHeapItem* SHeapLeaf::getRightChild()
{
	return nullptr;
}

const SSectionPos* SHeapLeaf::getMedian()
{
	return nullptr;
}

void SHeapLeaf::accept( KHeapItemVisitor* visitor )
{
	visitor->visitLeaf( this );
}

// ----------------------------------------------------------------------------

void KHeapItemVisitor::visitNode( SHeapNode* node )
{
	visitDefault( node );
}

void KHeapItemVisitor::visitLeaf( SHeapLeaf* leaf )
{
	visitDefault( leaf );
}

void KHeapItemVisitor::visitDefault( SHeapItem* /*item*/ )
{
	assert( !"unsupported item" );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

template< typename compare_by_1st_dim >
struct find_min_element
{
	section_positions_it operator()(
		section_positions_it begin,
		section_positions_it end )
	{
		auto result
			= std::min_element(
				begin
				, end
				, compare_by_1st_dim() );
		return result;
	}

};

template< typename compare_by_1st_dim >
struct find_max_element
{
	section_positions_it operator()(
		section_positions_it begin,
		section_positions_it end )
	{
		auto result
			= std::max_element(
				begin
				, end
				, compare_by_1st_dim() );
		return result;
	}

};

// ----------------------------------------------------------------------------

// returns true if 'item' is on the left/top side according to 'position'
template< typename comparator, typename position_t >
struct is_in_left_top_side_range
{
	explicit is_in_left_top_side_range( const position_t& position )
		: d_position( position )
	{
	}

	bool operator()( const SHeapItem* item ) const
	{
		const SSectionPos* sectpos = item->d_sectpos;
		const bool result = comparator()( sectpos, d_position );
		return result;
	}

	const position_t& d_position;
};

// returns true if 'item' is on the right/bottom side according to 'position'
template< typename comparator, typename position_t >
struct is_in_right_bottom_side_range
{
	explicit is_in_right_bottom_side_range( const position_t& position )
		: d_position( position )
	{
	}

	bool operator()( const SHeapItem* item ) const
	{
		const SSectionPos* sectpos = item->d_sectpos;
		const bool result = comparator()( d_position, sectpos );
		return result;
	}

	const position_t& d_position;
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

using sectpos_set_t = std::set< const SSectionPos* >;
using sectpos_set_it = sectpos_set_t::iterator;

template<
	typename is_in_1st_dim_range_t,
	typename compare_by_2nd_dim_t >
struct SCheckHeapConsistencyTraits
{
	using is_in_1st_dim_range = is_in_1st_dim_range_t;
	using compare_by_2nd_dim = compare_by_2nd_dim_t;
};

struct SCheckResult
{
	explicit SCheckResult( const section_positions_t& sect_positions )
		: d_consistent( true )
		, d_sectpos_set( sect_positions.begin(), sect_positions.end() )
	{
	}

	bool d_consistent;
	sectpos_set_t d_sectpos_set;
};

template< typename traits >
class KCheckHeapConsistency : public KHeapItemVisitor
{
	public:
		KCheckHeapConsistency(
			const SHeapNode* parent,
			EChildSide childSide,
			SCheckResult* result );

	public:
		static bool run(
			SHeapItem* heapRoot,
			const section_positions_t& sect_positions );

	public:
		void visitNode( SHeapNode* node ) override;
		void visitDefault( SHeapItem* item ) override;

	private:
		void checkPosAgainstParent( SHeapItem* item );
		void updatePointPosSet( SHeapItem* item );

	private:
		const SHeapNode* d_parent;
		const EChildSide d_childSide;

		SCheckResult* d_result;

};

// ----------------------------------------------------------------------------

template< typename traits >
KCheckHeapConsistency< traits >::KCheckHeapConsistency(
	const SHeapNode* parent,
	EChildSide childSide,
	SCheckResult* result )
	: d_parent( parent )
	, d_childSide( childSide )
	, d_result( result )
{
}

template< typename traits >
bool KCheckHeapConsistency< traits >::run(
	SHeapItem* heapRoot,
	const section_positions_t& sect_positions )
{
	bool result = true;
	#ifdef ENABLE_TREE_CHECKERS
	if ( heapRoot != nullptr )
	{
		SCheckResult checkResult( sect_positions );
		KCheckHeapConsistency< traits > checkConsistency( nullptr, NoParent, &checkResult );
		heapRoot->accept( &checkConsistency );
		result = checkResult.d_consistent && checkResult.d_sectpos_set.empty();
		assert( result ); // stopping assertion
	}
	#endif
	return result;
}

template< typename traits >
void KCheckHeapConsistency< traits >::visitNode( SHeapNode* node )
{
	visitDefault( node );

	SHeapItem* leftChild = node->getLeftChild();
	if ( leftChild )
	{
		KCheckHeapConsistency< traits > checkConsistency( node, LeftChild, d_result );
		leftChild->accept( &checkConsistency );
	}

	SHeapItem* rightChild = node->getRightChild();
	if ( rightChild )
	{
		KCheckHeapConsistency< traits > checkConsistency( node, RightChild, d_result );
		rightChild->accept( &checkConsistency );
	}
}

template< typename traits >
void KCheckHeapConsistency< traits >::visitDefault( SHeapItem* item )
{
	checkPosAgainstParent( item );
	updatePointPosSet( item );
}

template< typename traits >
void KCheckHeapConsistency< traits >::checkPosAgainstParent( SHeapItem* item )
{
	if ( d_parent )
	{
		bool consistent = true;
		const SSectionPos* parentPos = d_parent->d_sectpos;
		if ( typename traits::is_in_1st_dim_range( parentPos )( item ) )
		{
			const SSectionPos* median = d_parent->d_median;
			const SSectionPos* itemPos = item->d_sectpos;
			if ( d_childSide == LeftChild )
			{
				if ( ! typename traits::compare_by_2nd_dim()( itemPos, median ) && ( itemPos != median ) )
					consistent = false;
			}
			else
			{
				assert( d_childSide == RightChild );
				if ( ! typename traits::compare_by_2nd_dim()( median, itemPos ) )
					consistent = false;
			}
		}
		else
		{
			consistent = false;
		}

		if ( !consistent )
		{
			d_result->d_consistent = false;
			assert( !"inconsistent heap!" ); // stopping assertion
		}
	}
}

template< typename traits >
void KCheckHeapConsistency< traits >::updatePointPosSet( SHeapItem* item )
{
	sectpos_set_t& sectpos_set = d_result->d_sectpos_set;
	const SSectionPos* sectPos = item->d_sectpos;
	auto it = d_result->d_sectpos_set.find( sectPos );
	assert( it != sectpos_set.end() );
	sectpos_set.erase( it );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

template<
	typename find_subtree_root_t,
	typename compare_by_2nd_dim_t >
struct SHeapBuilderTraits
{
	using find_subtree_root = find_subtree_root_t;
	using compare_by_2nd_dim = compare_by_2nd_dim_t;
};

template< typename traits >
class KHeapBuilder
{
	public:
		KHeapBuilder() = default;

	public:
		SHeapItem* run(
			section_positions_it begin,
			section_positions_it end );

	private:
		SHeapItem* createItem(
			section_positions_it begin,
			section_positions_it end );

		SHeapItem* createNode(
			section_positions_it begin,
			section_positions_it end );

		SHeapItem* createLeaf(
			const SSectionPos* sectpos );

	private:
		void prepareSubtreeChildrenRange(
			section_positions_it raw_begin,
			section_positions_it it_subtree_root,
			section_positions_it raw_end,
			section_positions_it* begin,
			section_positions_it* end );

};

// ----------------------------------------------------------------------------

template< typename traits >
SHeapItem* KHeapBuilder< traits >::run(
	section_positions_it begin,
	section_positions_it end )
{
	std::sort( begin, end, typename traits::compare_by_2nd_dim() );
	assert( std::adjacent_find( begin, end ) == end ); // items should be unique

	SHeapItem* root = createItem( begin, end );
	return root;
}

// ----------------------------------------------------------------------------

template< typename traits >
SHeapItem* KHeapBuilder< traits >::createItem(
	section_positions_it begin,
	section_positions_it end )
{
	SHeapItem* result = nullptr;
	const std::size_t sectPosCount = std::distance( begin, end );
	if ( 1 < sectPosCount )
	{
		result = createNode( begin, end);
	}
	else if ( sectPosCount == 1 )
	{
		const SSectionPos* sectpos = *begin;
		result = createLeaf( sectpos );
	}
	return result;
}

template< typename traits >
SHeapItem* KHeapBuilder< traits >::createNode(
	section_positions_it raw_begin,
	section_positions_it raw_end )
{
	// find sectpos according to 1st dim (minimal coord for left/top side or
	// maximal for right/bottom), it will be hold in this subtree root
	auto it_subtree_root
		= typename traits::find_subtree_root()( raw_begin, raw_end );
	const SSectionPos* subtreeRootSectPos = *it_subtree_root;
	section_positions_it begin;
	section_positions_it end;
	prepareSubtreeChildrenRange( raw_begin, it_subtree_root, raw_end, &begin, &end );

	assert( utils::is_sorted( begin, end, typename traits::compare_by_2nd_dim() ) );

	auto median_it = utils::get_median( begin, end );
	const SSectionPos* medianSectPos = *median_it;

	auto node = new SHeapNode( subtreeRootSectPos, medianSectPos );

	auto lend = median_it + 1;
	node->d_leftChild = createItem( begin, lend );

	auto rbegin = lend;
	node->d_rightChild = createItem( rbegin, end );

	return node;
}

template< typename traits >
SHeapItem* KHeapBuilder< traits >::createLeaf( const SSectionPos* sectpos )
{
	return new SHeapLeaf( sectpos );
}

// ----------------------------------------------------------------------------

template< typename traits >
void KHeapBuilder< traits >::prepareSubtreeChildrenRange(
	section_positions_it raw_begin,
	section_positions_it it_subtree_root,
	section_positions_it raw_end,
	section_positions_it* begin,
	section_positions_it* end )
{
	// sectpos at it_subtree_root is already stored and needless, we need continuous
	// range of subtree range, so move other sect_positions to overwrite it and make
	// continuous range <begin,end)
	const std::size_t diff_from_begin = std::distance( raw_begin, it_subtree_root );
	const std::size_t diff_to_end = std::distance( it_subtree_root, raw_end - 1 );
	if ( diff_from_begin <= diff_to_end )
	{
		*begin = raw_begin + 1;
		if ( diff_from_begin != 0 )
			std::copy_backward( raw_begin, it_subtree_root, it_subtree_root + 1 );
		*end = raw_end;
	}
	else
	{
		*begin = raw_begin;
		if ( diff_to_end != 0 )
			std::copy( it_subtree_root + 1, raw_end, it_subtree_root );
		*end = raw_end - 1;
	}
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

struct SIntervalTreeItem;
struct SIntervalTreeNode;
struct SIntervalTreeLeaf;

class KIntervalTreeItemVisitor
{
	protected:
		KIntervalTreeItemVisitor() = default;

	public:
		virtual ~KIntervalTreeItemVisitor() = default;

	public:
		virtual void visitNode( SIntervalTreeNode* node );
		virtual void visitLeaf( SIntervalTreeLeaf* leaf );
		virtual void visitDefault( SIntervalTreeItem* item );

};

// ----------------------------------------------------------------------------

struct SIntervalTreeItem
{
	protected:
		SIntervalTreeItem() = default;

	public:
		virtual ~SIntervalTreeItem() = default;

	public:
		virtual SIntervalTreeItem* getLeftChild() = 0;
		virtual SIntervalTreeItem* getRightChild() = 0;

		virtual void accept( KIntervalTreeItemVisitor* visitor ) = 0;

};

// ----------------------------------------------------------------------------

struct SIntervalTreeNode : public SIntervalTreeItem
{
	public:
		explicit SIntervalTreeNode( const SSectionPos* medSectPos );
		~SIntervalTreeNode() override;

	public:
		SIntervalTreeItem* getLeftChild() override;
		SIntervalTreeItem* getRightChild() override;

		void accept( KIntervalTreeItemVisitor* visitor ) override;

	public:
		const SSectionPos* d_medSectPos;
		SHeapItem* d_medSectPositionsOnLeftTop;
		SHeapItem* d_medSectPositionsOnRightBottom;
		SIntervalTreeItem* d_leftChild;
		SIntervalTreeItem* d_rightChild;

};

SIntervalTreeNode::SIntervalTreeNode( const SSectionPos* medSectPos )
	: d_medSectPos( medSectPos )
	, d_medSectPositionsOnLeftTop(nullptr )
	, d_medSectPositionsOnRightBottom( nullptr )
	, d_leftChild( nullptr )
	, d_rightChild( nullptr )
{
}

SIntervalTreeNode::~SIntervalTreeNode()
{
	delete d_medSectPositionsOnLeftTop;
	delete d_medSectPositionsOnRightBottom;
	delete d_leftChild;
	delete d_rightChild;
}

SIntervalTreeItem* SIntervalTreeNode::getLeftChild()
{
	return d_leftChild;
}

SIntervalTreeItem* SIntervalTreeNode::getRightChild()
{
	return d_rightChild;
}

void SIntervalTreeNode::accept( KIntervalTreeItemVisitor* visitor )
{
	visitor->visitNode( this );
}

// ----------------------------------------------------------------------------

struct SIntervalTreeLeaf : public SIntervalTreeItem
{
	public:
		SIntervalTreeLeaf(
			const SSectionPos* beginSectPos,
			const SSectionPos* endSectPos );

	public:
		SIntervalTreeItem* getLeftChild() override;
		SIntervalTreeItem* getRightChild() override;

		void accept( KIntervalTreeItemVisitor* visitor ) override;

	public:
		const SSectionPos* d_beginSectPos;
		const SSectionPos* d_endSectPos;

};

SIntervalTreeLeaf::SIntervalTreeLeaf(
	const SSectionPos* beginSectPos,
	const SSectionPos* endSectPos )
	: d_beginSectPos( beginSectPos )
	, d_endSectPos( endSectPos )
{
	assert( KSegmentsManager::isSection( beginSectPos, endSectPos ) );
}

SIntervalTreeItem* SIntervalTreeLeaf::getLeftChild()
{
	return nullptr;
}

SIntervalTreeItem* SIntervalTreeLeaf::getRightChild()
{
	return nullptr;
}

void SIntervalTreeLeaf::accept( KIntervalTreeItemVisitor* visitor )
{
	visitor->visitLeaf( this );
}

// ----------------------------------------------------------------------------

void KIntervalTreeItemVisitor::visitNode( SIntervalTreeNode* node )
{
	visitDefault( node );
}

void KIntervalTreeItemVisitor::visitLeaf( SIntervalTreeLeaf* leaf )
{
	visitDefault( leaf );
}

void KIntervalTreeItemVisitor::visitDefault( SIntervalTreeItem* /*item*/ )
{
	assert( !"unsupported item" );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KTraverseHeap : public KHeapItemVisitor
{
	public:
		explicit KTraverseHeap( sectpos_set_t* sectpos_set );

	public:
		void visitNode( SHeapNode* node ) override;
		void visitDefault( SHeapItem* item ) override;

	private:
		sectpos_set_t* d_sectpos_set;

};

// ----------------------------------------------------------------------------

KTraverseHeap::KTraverseHeap( sectpos_set_t* sectpos_set )
	: d_sectpos_set( sectpos_set )
{
}

void KTraverseHeap::visitNode( SHeapNode* node )
{
	visitDefault( node );

	SHeapItem* leftChild = node->getLeftChild();
	if ( leftChild )
		leftChild->accept( this );

	SHeapItem* rightChild = node->getRightChild();
	if ( rightChild )
		rightChild->accept( this );
}

void KTraverseHeap::visitDefault( SHeapItem* item )
{
	const SSectionPos* itemPos = item->d_sectpos;
	auto it = d_sectpos_set->find( itemPos );
	assert( it != d_sectpos_set->end() );
	d_sectpos_set->erase( it );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

using sectpos_set_t = std::set< const SSectionPos* >;
using sectpos_set_it = sectpos_set_t::iterator;

template< typename compare_by_1st_dim_t >
struct SCheckIntervalTreeConsistencyTraits
{
	using compare_by_1st_dim = compare_by_1st_dim_t;
};

template< typename traits >
class KCheckIntervalTreeConsistency : public KIntervalTreeItemVisitor
{
	public:
		KCheckIntervalTreeConsistency(
			const SIntervalTreeNode* parent,
			EChildSide childSide,
			SCheckResult* result );

	public:
		static bool run(
			SIntervalTreeItem* root,
			const section_positions_t& sect_positions );

	public:
		void visitNode( SIntervalTreeNode* node ) override;
		void visitLeaf( SIntervalTreeLeaf* leaf ) override;

	private:
		void checkPosAgainstParent( const SSectionPos* itemPos );
		void checkNodeHeaps( SIntervalTreeNode* node );
		void updatePointPosSet( const SSectionPos* sectPos );

	private:
		const SIntervalTreeNode* d_parent;
		const EChildSide d_childSide;

		SCheckResult* d_result;

};

// ----------------------------------------------------------------------------

template< typename traits >
KCheckIntervalTreeConsistency< traits >::KCheckIntervalTreeConsistency(
	const SIntervalTreeNode* parent,
	EChildSide childSide,
	SCheckResult* result )
	: d_parent( parent )
	, d_childSide( childSide )
	, d_result( result )
{
}

template< typename traits >
bool KCheckIntervalTreeConsistency< traits >::run(
	SIntervalTreeItem* root,
	const section_positions_t& sect_positions )
{
	bool result = true;
	#ifdef ENABLE_TREE_CHECKERS
	if ( root != nullptr )
	{
		SCheckResult checkResult( sect_positions );
		KCheckIntervalTreeConsistency< traits > checkConsistency( nullptr, NoParent, &checkResult );
		root->accept( &checkConsistency );
		result = checkResult.d_consistent && checkResult.d_sectpos_set.empty();
		assert( result ); // stopping assertion
	}
	#endif
	return result;
}

template< typename traits >
void KCheckIntervalTreeConsistency< traits >::visitNode( SIntervalTreeNode* node )
{
	const SSectionPos* itemPos = node->d_medSectPos;
	checkPosAgainstParent( itemPos );
	checkNodeHeaps( node );

	SIntervalTreeItem* leftChild = node->getLeftChild();
	if ( leftChild )
	{
		KCheckIntervalTreeConsistency< traits > checkConsistency( node, LeftChild, d_result );
		leftChild->accept( &checkConsistency );
	}

	SIntervalTreeItem* rightChild = node->getRightChild();
	if ( rightChild )
	{
		KCheckIntervalTreeConsistency< traits > checkConsistency( node, RightChild, d_result );
		rightChild->accept( &checkConsistency );
	}
}

template< typename traits >
void KCheckIntervalTreeConsistency< traits >::visitLeaf( SIntervalTreeLeaf* leaf )
{
	const SSectionPos* beginSectPos = leaf->d_beginSectPos;
	checkPosAgainstParent( beginSectPos );
	updatePointPosSet( beginSectPos );

	const SSectionPos* endSectPos = leaf->d_endSectPos;
	checkPosAgainstParent( endSectPos );
	updatePointPosSet( endSectPos );
}

template< typename traits >
void KCheckIntervalTreeConsistency< traits >::checkPosAgainstParent( const SSectionPos* itemPos )
{
	if ( d_parent )
	{
		bool consistent = true;
		const SSectionPos* parentPos = d_parent->d_medSectPos;
		if ( d_childSide == LeftChild )
		{
			if ( ! typename traits::compare_by_1st_dim()( itemPos, parentPos ) )
				consistent = false;
		}
		else
		{
			assert( d_childSide == RightChild );
			if ( ! typename traits::compare_by_1st_dim()( parentPos, itemPos ) )
				consistent = false;
		}

		if ( !consistent )
		{
			d_result->d_consistent = false;
			assert( !"inconsistent heap!" ); // stopping assertion
		}
	}
}

template< typename traits >
void KCheckIntervalTreeConsistency< traits >::checkNodeHeaps( SIntervalTreeNode* node )
{
	KTraverseHeap traverseHeap( &d_result->d_sectpos_set );

	SHeapItem* medSectPositionsOnLeftTop = node->d_medSectPositionsOnLeftTop;
	if ( medSectPositionsOnLeftTop )
		medSectPositionsOnLeftTop->accept( &traverseHeap );

	SHeapItem* medSectPositionsOnRightBottom = node->d_medSectPositionsOnRightBottom;
	if ( medSectPositionsOnRightBottom )
		medSectPositionsOnRightBottom->accept( &traverseHeap );
}

template< typename traits >
void KCheckIntervalTreeConsistency< traits >::updatePointPosSet( const SSectionPos* sectPos )
{
	sectpos_set_t& sectpos_set = d_result->d_sectpos_set;
	auto it = d_result->d_sectpos_set.find( sectPos );
	assert( it != sectpos_set.end() );
	sectpos_set.erase( it );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

template<
	typename compare_by_1st_dim_t,
	typename compare_by_2nd_dim_t >
struct SIntervalTreeBuilderTraits
{
	using compare_by_1st_dim = compare_by_1st_dim_t;
	using compare_by_2nd_dim = compare_by_2nd_dim_t;
};


template< typename traits >
class KIntervalTreeBuilder
{
	public:
		explicit KIntervalTreeBuilder( const KSegmentsManager& segmentsManager );

	public:
		SIntervalTreeItem* run( const section_positions_t& sect_positions_by_1st_dim );

	private:
		SIntervalTreeItem* createItem(
			const section_positions_t& sect_positions_by_1st_dim );

		SIntervalTreeItem* createNode(
			const section_positions_t& sect_positions_by_1st_dim );

		SIntervalTreeItem* createLeaf(
			const SSectionPos* beginSectPos,
			const SSectionPos* endSectPos );

	private:
		void prepareNodeLeftSide(
			section_positions_cit begin,
			section_positions_cit end,
			const SSectionPos* medianSectPos,
			section_positions_t* medSectPositionsOnLeftTop,
			section_positions_t* sectPositionsOutOnLeftTop ) const;
		SHeapItem* createHeapMedSectPositionsOnLeftTop(
			section_positions_t* medSectPositionsOnLeftTop ) const;

		void prepareNodeRightSide(
			section_positions_cit begin,
			section_positions_cit end,
			const SSectionPos* medianSectPos,
			section_positions_t* medSectPositionsOnRightBottom,
			section_positions_t* sectPositionsOutOnRightBottom ) const;
		SHeapItem* createHeapMedSectPositionsOnRightBottom(
			section_positions_t* medSectPositionsOnRightBottom ) const;

	private:
		const KSegmentsManager& d_segmentsManager;

};

// ----------------------------------------------------------------------------

template< typename traits >
KIntervalTreeBuilder< traits >::KIntervalTreeBuilder( const KSegmentsManager& segmentsManager )
	: d_segmentsManager( segmentsManager )
{
}

template< typename traits >
SIntervalTreeItem* KIntervalTreeBuilder< traits >::run( const section_positions_t& sect_positions_by_1st_dim )
{
	SIntervalTreeItem* root = createItem( sect_positions_by_1st_dim );
	return root;
}

// ----------------------------------------------------------------------------

template< typename traits >
SIntervalTreeItem* KIntervalTreeBuilder< traits >::createItem(
	const section_positions_t& sect_positions_by_1st_dim )
{
	SIntervalTreeItem* result = nullptr;
	const std::size_t sectPosCount = sect_positions_by_1st_dim.size();
	if ( 2 < sectPosCount )
	{
		result = createNode( sect_positions_by_1st_dim );
	}
	else if ( sectPosCount == 2 )
	{
		const SSectionPos* beginSectPos = sect_positions_by_1st_dim.front();
		const SSectionPos* endSectPos = sect_positions_by_1st_dim.back();
		result = createLeaf( beginSectPos, endSectPos );
	}
	else
	{
		assert( sectPosCount == 0 );
	}
	return result;
}

template< typename traits >
SIntervalTreeItem* KIntervalTreeBuilder< traits >::createNode( const section_positions_t& sect_positions_by_1st_dim )
{
	auto begin = sect_positions_by_1st_dim.begin();
	auto end = sect_positions_by_1st_dim.end();
	auto median_node_it = utils::get_median( begin, end );
	assert( median_node_it != sect_positions_by_1st_dim.end() );

	const SSectionPos* medianSectPos = *median_node_it;
	auto node = new SIntervalTreeNode( medianSectPos );

	auto lbegin = begin;
	auto lend = median_node_it;
	section_positions_t medSectPositionsOnLeftTop( 1, medianSectPos );
	section_positions_t sectPositionsOutOnLeftTop;
	prepareNodeLeftSide(
		lbegin,
		lend,
		medianSectPos,
		&medSectPositionsOnLeftTop,
		&sectPositionsOutOnLeftTop );
	node->d_medSectPositionsOnLeftTop = createHeapMedSectPositionsOnLeftTop( &medSectPositionsOnLeftTop );
	node->d_leftChild = createItem( sectPositionsOutOnLeftTop );

	section_positions_cit rbegin = median_node_it + 1;
	section_positions_cit rend = sect_positions_by_1st_dim.end();
	section_positions_t medSectPositionsOnRightBottom;
	section_positions_t sectPositionsOutOnRightBottom;
	prepareNodeRightSide(
		rbegin,
		rend,
		medianSectPos,
		&medSectPositionsOnRightBottom,
		&sectPositionsOutOnRightBottom );
	node->d_medSectPositionsOnRightBottom = createHeapMedSectPositionsOnRightBottom( &medSectPositionsOnRightBottom );
	node->d_rightChild = createItem( sectPositionsOutOnRightBottom );

	return node;
}

template< typename traits >
SIntervalTreeItem* KIntervalTreeBuilder< traits >::createLeaf(
	const SSectionPos* beginSectPos,
	const SSectionPos* endSectPos )
{
	return new SIntervalTreeLeaf( beginSectPos, endSectPos );
}

// ----------------------------------------------------------------------------

template< typename traits >
void KIntervalTreeBuilder< traits >::prepareNodeLeftSide(
	section_positions_cit begin,
	section_positions_cit end,
	const SSectionPos* medianSectPos,
	section_positions_t* medSectPositionsOnLeftTop,
	section_positions_t* sectPositionsOutOnLeftTop ) const
{
	for ( auto it = begin
		; it != end
		; ++it )
	{
		const SSectionPos* sectpos = *it;
		const SSectionPos* endSectPos = d_segmentsManager.getSectionEndPos( sectpos );
		if ( typename traits::compare_by_1st_dim()( endSectPos, medianSectPos ) )
			sectPositionsOutOnLeftTop->push_back( sectpos );
		else
			medSectPositionsOnLeftTop->push_back( sectpos );
	}
}

template< typename traits >
SHeapItem* KIntervalTreeBuilder< traits >::createHeapMedSectPositionsOnLeftTop(
	section_positions_t* medSectPositionsOnLeftTop ) const
{
	#ifndef NDEBUG
	// medSectPositionsOnLeftTop may be changed while building heap, so keep copy for checker
	// (vide assertion at the end of routine)
	const section_positions_t medSectPositionsOnLeftTopCopy( *medSectPositionsOnLeftTop );
	#endif

	using builder_traits_t = SHeapBuilderTraits<
		find_min_element< typename traits::compare_by_1st_dim >
		, typename traits::compare_by_2nd_dim
		>;
	KHeapBuilder< builder_traits_t > heapBuilder;
	section_positions_it begin = medSectPositionsOnLeftTop->begin();
	section_positions_it end = medSectPositionsOnLeftTop->end();
	SHeapItem* heapRoot = heapBuilder.run( begin, end );

	using is_behind_parent_t = is_in_right_bottom_side_range<
		typename traits::compare_by_1st_dim,
		const SSectionPos* >;
	using checker_traits_t = SCheckHeapConsistencyTraits<
		is_behind_parent_t,
		typename traits::compare_by_2nd_dim >;
	assert( KCheckHeapConsistency< checker_traits_t >::run( heapRoot, medSectPositionsOnLeftTopCopy ) );

	return heapRoot;
}

template< typename traits >
void KIntervalTreeBuilder< traits >::prepareNodeRightSide(
	section_positions_cit begin,
	section_positions_cit end,
	const SSectionPos* medianSectPos,
	section_positions_t* medSectPositionsOnRightBottom,
	section_positions_t* sectPositionsOutOnRightBottom ) const
{
	for ( section_positions_cit it = begin
		; it != end
		; ++it )
	{
		const SSectionPos* sectpos = *it;
		const SSectionPos* beginSectPos = d_segmentsManager.getSectionBeginPos( sectpos );
		if ( typename traits::compare_by_1st_dim()( medianSectPos, beginSectPos ) )
			sectPositionsOutOnRightBottom->push_back( sectpos );
		else
			medSectPositionsOnRightBottom->push_back( sectpos );
	}
}

template< typename traits >
SHeapItem* KIntervalTreeBuilder< traits >::createHeapMedSectPositionsOnRightBottom(
	section_positions_t* medSectPositionsOnRightBottom ) const
{
	#ifndef NDEBUG
	// medSectPositionsOnRightBottom may be changed while building heap, so keep copy for checker
	// (vide assertion at the end of routine)
	const section_positions_t medSectPositionsOnRightBottomCopy( *medSectPositionsOnRightBottom );
	#endif

	using builder_traits_t = SHeapBuilderTraits<
		find_max_element< typename traits::compare_by_1st_dim >
		, typename traits::compare_by_2nd_dim
		>;
	KHeapBuilder< builder_traits_t > heapBuilder;
	section_positions_it begin = medSectPositionsOnRightBottom->begin();
	section_positions_it end = medSectPositionsOnRightBottom->end();
	SHeapItem* heapRoot = heapBuilder.run( begin, end );

	using is_in_front_of_parent_t = is_in_left_top_side_range<
		typename traits::compare_by_1st_dim,
		const SSectionPos* >;
	using checker_traits_t = SCheckHeapConsistencyTraits<
		is_in_front_of_parent_t,
		typename traits::compare_by_2nd_dim >;
	assert( KCheckHeapConsistency< checker_traits_t >::run( heapRoot, medSectPositionsOnRightBottomCopy ) );

	return heapRoot;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

template<
	typename is_in_1st_dim_range_t,
	typename compare_by_2nd_dim_t >
struct SFindSplitNodeTraits
{
	using is_in_1st_dim_range = is_in_1st_dim_range_t;
	using compare_by_2nd_dim = compare_by_2nd_dim_t;
};

template< typename traits >
class KFindSplitNode : public KHeapItemVisitor
{
	public:
		KFindSplitNode(
			const KViewportArea& viewportArea,
			const SViewportBorder& axis,
			const SViewportBorder& min2ndDimEdge,
			const SViewportBorder& max2ndDimEdge,
			sect_pos_ids_t* sectposids );

		SHeapItem* getResult();

	public:
		void visitNode( SHeapNode* node ) override;
		void visitLeaf( SHeapLeaf* leaf ) override;

	private:
		bool isIn1stDimRange( const SHeapItem* item ) const;
		bool isIn2ndDimRange( const SSectionPos* sectpos ) const;
		void tryAddItem( const SHeapNode* node );

	private:
		const KViewportArea& d_viewportArea;
		const SViewportBorder& d_axis;
		const SViewportBorder& d_min2ndDimEdge;
		const SViewportBorder& d_max2ndDimEdge;
		sect_pos_ids_t& d_sectposids;
		SHeapItem* d_result;

};

// ----------------------------------------------------------------------------

template< typename traits >
KFindSplitNode< traits >::KFindSplitNode(
	const KViewportArea& viewportArea,
	const SViewportBorder& axis,
	const SViewportBorder& min2ndDimEdge,
	const SViewportBorder& max2ndDimEdge,
	sect_pos_ids_t* sectposids )
	: d_viewportArea( viewportArea )
	, d_axis( axis )
	, d_min2ndDimEdge( min2ndDimEdge )
	, d_max2ndDimEdge( max2ndDimEdge )
	, d_sectposids( *sectposids )
	, d_result( nullptr )
{
}

template< typename traits >
SHeapItem* KFindSplitNode< traits >::getResult()
{
	return d_result;
}

template< typename traits >
void KFindSplitNode< traits >::visitNode( SHeapNode* node )
{
	if ( isIn1stDimRange( node ) )
	{
		const SSectionPos* median = node->d_median;
		if ( typename traits::compare_by_2nd_dim()( median, d_min2ndDimEdge ) )
		{
			SHeapItem* rightChild = node->getRightChild();
			if ( rightChild )
				rightChild->accept( this );
		}
		else if ( typename traits::compare_by_2nd_dim()( d_max2ndDimEdge, median ) )
		{
			SHeapItem* leftChild = node->getLeftChild();
			if ( leftChild )
				leftChild->accept( this );
		}
		else
		{
			assert( isIn2ndDimRange( median ) );
			d_result = node;
		}

		// even if median is not within 2nd dim range, then the subtree root
		// may be in viewport area
		if ( d_result != node )
			tryAddItem( node );
	}
}

template< typename traits >
void KFindSplitNode< traits >::visitLeaf( SHeapLeaf* leaf )
{
	if ( isIn1stDimRange( leaf ) )
	{
		const SSectionPos* sectpos = leaf->d_sectpos;
		if ( isIn2ndDimRange( sectpos ) )
		{
			d_result = leaf;
		}
	}
}

template< typename traits >
bool KFindSplitNode< traits >::isIn1stDimRange( const SHeapItem* item ) const
{
	/*
		item as subtree-root it is the minimal (for left/top side)
		or maximal (for right/bottom side) x/y-coord in this heap,
		so can find split-node only if it is range [item-root, axis]
		for left/top or [axis, item-root] for right/bottom, if not
		then we know for sure that all other nodes also aren't in
		the range
	*/
	const bool result = typename traits::is_in_1st_dim_range( d_axis )( item );
	return result;
}

template< typename traits >
bool KFindSplitNode< traits >::isIn2ndDimRange( const SSectionPos* sectpos ) const
{
	const bool result = typename traits::compare_by_2nd_dim()( d_min2ndDimEdge, sectpos )
		&& typename traits::compare_by_2nd_dim()( sectpos, d_max2ndDimEdge );
	return result;
}

template< typename traits >
void KFindSplitNode< traits >::tryAddItem( const SHeapNode* node )
{
	const SSectionPos* nodepos = node->d_sectpos;
	if ( isIn2ndDimRange( nodepos ) )
	{
		const sect_pos_id_t sectid = nodepos->d_id;
		d_sectposids.push_back( sectid );
	}
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

template< typename is_in_1st_dim_range_t >
struct SDumpSectPositionsTraits
{
	using is_in_1st_dim_range = is_in_1st_dim_range_t;
};


template< typename traits >
class KDumpSectPositions : public KHeapItemVisitor
{
	public:
		KDumpSectPositions(
			const SViewportBorder& axis,
			sect_pos_ids_t* sectposids );

	public:
		void visitNode( SHeapNode* node ) override;
		void visitDefault( SHeapItem* item ) override;

	private:
		const SViewportBorder& d_axis;
		sect_pos_ids_t& d_sectposids;

};

// ----------------------------------------------------------------------------

template< typename traits >
KDumpSectPositions< traits >::KDumpSectPositions(
	const SViewportBorder& axis,
	sect_pos_ids_t* sectposids )
	: d_axis( axis )
	, d_sectposids( *sectposids )
{
}

template< typename traits >
void KDumpSectPositions< traits >::visitNode( SHeapNode* node )
{
	visitDefault( node );

	SHeapItem* leftChild = node->getLeftChild();
	if ( leftChild )
		leftChild->accept( this );

	SHeapItem* rightChild = node->getRightChild();
	if ( rightChild )
		rightChild->accept( this );
}

template< typename traits >
void KDumpSectPositions< traits >::visitDefault( SHeapItem* item )
{
	if ( typename traits::is_in_1st_dim_range( d_axis )( item ) )
	{
		const SSectionPos* sectpos = item->d_sectpos;
		const sect_pos_id_t sectposid = sectpos->d_id;
		d_sectposids.push_back( sectposid );
	}
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

template<
	typename is_in_1st_dim_range_t,
	typename compare_by_2nd_dim_t >
struct SSelectMedSectPositionsTraits
{
	using is_in_1st_dim_range = is_in_1st_dim_range_t;
	using compare_by_2nd_dim = compare_by_2nd_dim_t;
};


template< typename traits >
class KSelectMedSectPositions : public KHeapItemVisitor
{
	public:
		KSelectMedSectPositions(
			const KViewportArea& viewportArea,
			const SViewportBorder& axis,
			const SViewportBorder& min2ndDimEdge,
			const SViewportBorder& max2ndDimEdge,
			sect_pos_ids_t* sectposids );

	public:
		void visitNode( SHeapNode* node ) override;
		void visitDefault( SHeapItem* item ) override;

	private:
		void traverseLeftSubtree( SHeapNode* subtreeRoot );
		void traverseRightSubtree( SHeapNode* subtreeRoot );
		void dumpInnerSubtree( SHeapItem* root, EChildSide subtreeSide );

		bool isIn1stDimRange( SHeapItem* item ) const;
		bool isItemInArea( SHeapItem* item ) const;
		void addItem( SHeapItem* item );

	private:
		const KViewportArea& d_viewportArea;
		const SViewportBorder& d_axis;
		const SViewportBorder& d_min2ndDimEdge;
		const SViewportBorder& d_max2ndDimEdge;
		using TDumpSectPositionsTraits = SDumpSectPositionsTraits< typename traits::is_in_1st_dim_range >;
		KDumpSectPositions< TDumpSectPositionsTraits > d_dumpSectPositions;
		sect_pos_ids_t& d_sectposids;

};

// ----------------------------------------------------------------------------

template< typename traits >
KSelectMedSectPositions< traits >::KSelectMedSectPositions(
	const KViewportArea& viewportArea,
	const SViewportBorder& axis,
	const SViewportBorder& min2ndDimEdge,
	const SViewportBorder& max2ndDimEdge,
	sect_pos_ids_t* sectposids )
	: d_viewportArea( viewportArea )
	, d_axis( axis )
	, d_min2ndDimEdge( min2ndDimEdge )
	, d_max2ndDimEdge( max2ndDimEdge )
	, d_dumpSectPositions( axis, sectposids )
	, d_sectposids( *sectposids )
{
}

template< typename traits >
void KSelectMedSectPositions< traits >::visitNode( SHeapNode* node )
{
	if ( isItemInArea( node ) )
		addItem( node );
	traverseLeftSubtree( node );
	traverseRightSubtree( node );
}

template< typename traits >
void KSelectMedSectPositions< traits >::visitDefault( SHeapItem* item )
{
	if ( isItemInArea( item ) )
		addItem( item );
}

template< typename traits >
void KSelectMedSectPositions< traits >::traverseLeftSubtree( SHeapNode* subtreeRoot )
{
	SHeapItem* item = subtreeRoot->getLeftChild();
	while ( ( item != nullptr ) && isIn1stDimRange( item ) )
	{
		const SSectionPos* sectpos = item->d_sectpos;
		if ( typename traits::compare_by_2nd_dim()( d_min2ndDimEdge, sectpos ) )
			addItem( item );

		const SSectionPos* median = item->getMedian();
		if ( median )
		{
			if ( typename traits::compare_by_2nd_dim()( d_min2ndDimEdge, median ) )
			{
				assert( typename traits::compare_by_2nd_dim()( median, d_max2ndDimEdge ) );
				dumpInnerSubtree( item, RightChild );
				item = item->getLeftChild();
			}
			else
			{
				item = item->getRightChild();
			}
		}
		else
		{
			// it is leaf, stop the loop
			item = nullptr;
		}
	}
}

template< typename traits >
void KSelectMedSectPositions< traits >::traverseRightSubtree( SHeapNode* subtreeRoot )
{
	SHeapItem* item = subtreeRoot->getRightChild();
	while ( ( item != nullptr ) && isIn1stDimRange( item ) )
	{
		const SSectionPos* sectpos = item->d_sectpos;
		if ( typename traits::compare_by_2nd_dim()( sectpos, d_max2ndDimEdge ) )
			addItem( item );

		const SSectionPos* median = item->getMedian();
		if ( median )
		{
			if ( typename traits::compare_by_2nd_dim()( median, d_max2ndDimEdge ) )
			{
				assert( typename traits::compare_by_2nd_dim()( d_min2ndDimEdge, median ) );
				dumpInnerSubtree( item, LeftChild );
				item = item->getRightChild();
			}
			else
			{
				item = item->getLeftChild();
			}
		}
		else
		{
			// it is leaf, stop the loop
			item = nullptr;
		}
	}
}

template< typename traits >
void KSelectMedSectPositions< traits >::dumpInnerSubtree(
	SHeapItem* root,
	EChildSide subtreeSide )
{
	SHeapItem* subtreeRoot = subtreeSide == RightChild ? root->getRightChild() : root->getLeftChild();
	if ( subtreeRoot )
		subtreeRoot->accept( &d_dumpSectPositions );
}

template< typename traits >
bool KSelectMedSectPositions< traits >::isIn1stDimRange( SHeapItem* item ) const
{
	const bool result = typename traits::is_in_1st_dim_range( d_axis )( item );
	return result;
}

template< typename traits >
bool KSelectMedSectPositions< traits >::isItemInArea( SHeapItem* item ) const
{
	bool result = false;
	if ( isIn1stDimRange( item ) )
	{
		const SSectionPos* sectpos = item->d_sectpos;
		if ( typename traits::compare_by_2nd_dim()( d_min2ndDimEdge, sectpos )
			&& typename traits::compare_by_2nd_dim()( sectpos, d_max2ndDimEdge ) )
		{
			result = true;
		}
	}
	return result;
}

template< typename traits >
void KSelectMedSectPositions< traits >::addItem( SHeapItem* item )
{
	assert( isItemInArea( item ) );
	const sect_pos_id_t sectposid = item->d_sectpos->d_id;
	d_sectposids.push_back( sectposid );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


template<
	typename compare_by_1st_dim_t,
	typename is_in_front_of_1st_dim_axis_t,
	typename is_behind_1st_dim_axis_t,
	typename compare_by_2nd_dim_t >
struct SSelectSectPositionsTraits
{
	using compare_by_1st_dim = compare_by_1st_dim_t;
	using is_in_front_of_1st_dim_axis = is_in_front_of_1st_dim_axis_t;
	using is_behind_1st_dim_axis = is_behind_1st_dim_axis_t;
	using compare_by_2nd_dim = compare_by_2nd_dim_t;
};


template< typename traits >
class KSelectSectPositions : public KIntervalTreeItemVisitor
{
	public:
		KSelectSectPositions(
			const KViewportArea& viewportArea,
			const SViewportBorder& axis,
			const SViewportBorder& min2ndDimEdge,
			const SViewportBorder& max2ndDimEdge,
			sect_pos_ids_t* sectposids );

	public:
		void visitNode( SIntervalTreeNode* node ) override;
		void visitLeaf( SIntervalTreeLeaf* leaf ) override;

	private:
		void traverseLeftTopMedSectPositions( SHeapItem* medSectPositionsRoot );
		void traverseRightBottomMedSectPositions( SHeapItem* medSectPositionsRoot );

	private:
		const KViewportArea& d_viewportArea;
		const SViewportBorder& d_axis;
		const SViewportBorder& d_min2ndDimEdge;
		const SViewportBorder& d_max2ndDimEdge;
		sect_pos_ids_t& d_sectposids;

};

// ----------------------------------------------------------------------------

template< typename traits >
KSelectSectPositions< traits >::KSelectSectPositions(
	const KViewportArea& viewportArea,
	const SViewportBorder& axis,
	const SViewportBorder& min2ndDimEdge,
	const SViewportBorder& max2ndDimEdge,
	sect_pos_ids_t* sectposids )
	: d_viewportArea( viewportArea )
	, d_axis( axis )
	, d_min2ndDimEdge( min2ndDimEdge )
	, d_max2ndDimEdge( max2ndDimEdge )
	, d_sectposids( *sectposids )
{
}

template< typename traits >
void KSelectSectPositions< traits >::visitNode( SIntervalTreeNode* node )
{
	const SSectionPos* medSectPos = node->d_medSectPos;
	if ( typename traits::compare_by_1st_dim()( d_axis, medSectPos ) )
	{
		SHeapItem* medSectPositionsOnLeftTopRoot = node->d_medSectPositionsOnLeftTop;
		if ( medSectPositionsOnLeftTopRoot )
			traverseLeftTopMedSectPositions( medSectPositionsOnLeftTopRoot );
		SIntervalTreeItem* leftChild = node->d_leftChild;
		if ( leftChild )
			leftChild->accept( this );
	}
	else
	{
		SHeapItem* medSectPositionsOnRightBottomRoot = node->d_medSectPositionsOnRightBottom;
		if ( medSectPositionsOnRightBottomRoot )
			traverseRightBottomMedSectPositions( medSectPositionsOnRightBottomRoot );
		SIntervalTreeItem* rightChild = node->d_rightChild;
		if ( rightChild )
			rightChild->accept( this );
	}
}

template< typename traits >
void KSelectSectPositions< traits >::visitLeaf( SIntervalTreeLeaf* leaf )
{
	const SSectionPos* beginSectPos = leaf->d_beginSectPos;
	const SSectionPos* endSectPos = leaf->d_endSectPos;
	assert( KSegmentsManager::isSection( beginSectPos, endSectPos ) );
	if ( typename traits::compare_by_1st_dim()( beginSectPos, d_axis )
		&& typename traits::compare_by_1st_dim()( d_axis, endSectPos )
		&& typename traits::compare_by_2nd_dim()( d_min2ndDimEdge, beginSectPos )
		&& typename traits::compare_by_2nd_dim()( beginSectPos, d_max2ndDimEdge ) )
	{
		// add only id of the beginning of section, it is enough to
		// draw the whole section in the next stage
		const sect_pos_id_t sectposid = beginSectPos->d_id;
		d_sectposids.push_back( sectposid );
	}
}

template< typename traits >
void KSelectSectPositions< traits >::traverseLeftTopMedSectPositions( SHeapItem* medSectPositionsRoot )
{
	using SFindLeftTopSplitNodeTraits = SFindSplitNodeTraits<
		typename traits::is_in_front_of_1st_dim_axis,
		typename traits::compare_by_2nd_dim >;
	KFindSplitNode< SFindLeftTopSplitNodeTraits > findSplitNode(
		d_viewportArea,
		d_axis,
		d_min2ndDimEdge,
		d_max2ndDimEdge,
		&d_sectposids );
	medSectPositionsRoot->accept( &findSplitNode );
	SHeapItem* splitNode = findSplitNode.getResult();
	if ( splitNode )
	{
		using SSelectMedLeftTopSectPositionsTraits = SSelectMedSectPositionsTraits<
			typename traits::is_in_front_of_1st_dim_axis,
			typename traits::compare_by_2nd_dim >;
		KSelectMedSectPositions< SSelectMedLeftTopSectPositionsTraits > selectMedSectPositions(
			d_viewportArea,
			d_axis,
			d_min2ndDimEdge,
			d_max2ndDimEdge,
			&d_sectposids );
		splitNode->accept( &selectMedSectPositions );
	}
}

template< typename traits >
void KSelectSectPositions< traits >::traverseRightBottomMedSectPositions( SHeapItem* medSectPositionsRoot )
{
	using SFindRightBottomSplitNodeTraits = SFindSplitNodeTraits<
		typename traits::is_behind_1st_dim_axis,
		typename traits::compare_by_2nd_dim >;
	KFindSplitNode< SFindRightBottomSplitNodeTraits > findSplitNode(
		d_viewportArea,
		d_axis,
		d_min2ndDimEdge,
		d_max2ndDimEdge,
		&d_sectposids );
	medSectPositionsRoot->accept( &findSplitNode );
	SHeapItem* splitNode = findSplitNode.getResult();
	if ( splitNode )
	{
		using SSelectMedRightBottomSectPositionsTraits = SSelectMedSectPositionsTraits<
			typename traits::is_behind_1st_dim_axis,
			typename traits::compare_by_2nd_dim >;
		KSelectMedSectPositions< SSelectMedRightBottomSectPositionsTraits > selectMedSectPositions(
			d_viewportArea,
			d_axis,
			d_min2ndDimEdge,
			d_max2ndDimEdge,
			&d_sectposids );
		splitNode->accept( &selectMedSectPositions );
	}
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KIntervalTree::Impl
{
	public:
		Impl( const KSegmentsManager& segmentsManager );
		~Impl();

	public:
		template< typename traits >
		SIntervalTreeItem* create( EOrientation orientation );

		template< typename traits >
		void selectSectPositions(
			const KViewportArea& viewportArea,
			const SViewportBorder& axis,
			const SViewportBorder& min2ndDimEdge,
			const SViewportBorder& max2ndDimEdge,
			SIntervalTreeItem* root,
			sect_pos_ids_t* sectposids ) const;

		void selectCrossSections(
			const KViewportArea& viewportArea,
			const sect_pos_ids_t& crosssectposids,
			sect_pos_ids_t* sectposids ) const;

	public:
		const KSegmentsManager& d_segmentsManager;
		SIntervalTreeItem* d_horzRoot;
		SIntervalTreeItem* d_vertRoot;
};

KIntervalTree::Impl::Impl( const KSegmentsManager& segmentsManager )
	: d_segmentsManager( segmentsManager )
	, d_horzRoot( nullptr )
	, d_vertRoot( nullptr )
{
}

KIntervalTree::Impl::~Impl()
{
	delete d_horzRoot;
	delete d_vertRoot;
}

template< typename traits >
SIntervalTreeItem* KIntervalTree::Impl::create( EOrientation orientation )
{
	SIntervalTreeItem* treeRoot = nullptr;
	section_positions_t sect_positions;
	if ( d_segmentsManager.getSectPositions( orientation, &sect_positions ) )
	{
		std::sort( sect_positions.begin(), sect_positions.end(), typename traits::compare_by_1st_dim() );
		assert( std::adjacent_find(
			sect_positions.begin(), sect_positions.end() ) == sect_positions.end() ); // items should be unique
		KIntervalTreeBuilder< traits> treeBuilder( d_segmentsManager );
		treeRoot = treeBuilder.run( sect_positions );
		using checker_traits_t = SCheckIntervalTreeConsistencyTraits< typename traits::compare_by_1st_dim >;
		assert( KCheckIntervalTreeConsistency< checker_traits_t >::run( treeRoot, sect_positions ) );

	}
	return treeRoot;
}

template< typename traits >
void KIntervalTree::Impl::selectSectPositions(
	const KViewportArea& viewportArea,
	const SViewportBorder& axis,
	const SViewportBorder& min2ndDimEdge,
	const SViewportBorder& max2ndDimEdge,
	SIntervalTreeItem* root,
	sect_pos_ids_t* sectposids ) const
{
	if ( root )
	{
		KSelectSectPositions< traits > selectSectPositions(
			viewportArea,
			axis,
			min2ndDimEdge,
			max2ndDimEdge,
			sectposids );
		root->accept( &selectSectPositions );
	}
}

void KIntervalTree::Impl::selectCrossSections(
	const KViewportArea& viewportArea,
	const sect_pos_ids_t& crosssectposids,
	sect_pos_ids_t* sectposids ) const
{
	const SViewportBorder& rightEdge = viewportArea.getRightEdge();
	const coord_t right = rightEdge.d_border.x;

	const SViewportBorder& bottomEdge = viewportArea.getBottomEdge();
	const coord_t bottom = bottomEdge.d_border.y;

	for ( const sect_pos_id_t sectposid : crosssectposids )
	{
		const SPoint& sectionRectRightBottomCorner = d_segmentsManager.getSectionCrossPoint( sectposid );
		if ( ( right < sectionRectRightBottomCorner.x ) && ( bottom < sectionRectRightBottomCorner.y ) )
		{
			sectposids->push_back( sectposid );
		}
	}
}

// ----------------------------------------------------------------------------

KIntervalTree::KIntervalTree( const KSegmentsManager& segmentsManager )
	: impl( new Impl( segmentsManager ) )
{
	using SHorizontalTreeTraits = SIntervalTreeBuilderTraits< utils::compare_by_x, utils::compare_by_y >;
	impl->d_horzRoot = impl->create< SHorizontalTreeTraits >( Horizontal );

	using SVerticalTreeTraits = SIntervalTreeBuilderTraits< utils::compare_by_y, utils::compare_by_x >;
	impl->d_vertRoot = impl->create< SVerticalTreeTraits >( Vertical );
}

KIntervalTree::~KIntervalTree()
{
	delete impl;
}

void KIntervalTree::selectSectPositions(
	const KViewportArea& viewportArea,
	sect_pos_ids_t* sectposids ) const
{
	// horizontal-axis tree
	using is_in_front_of_horz_axis = is_in_left_top_side_range< utils::compare_by_x, SViewportBorder >;
	using is_behind_horz_axis = is_in_right_bottom_side_range< utils::compare_by_x, SViewportBorder >;

	using SSelectHorzSectPositionsTraits = SSelectSectPositionsTraits<
		utils::compare_by_x,
		is_in_front_of_horz_axis,
		is_behind_horz_axis,
		utils::compare_by_y >;

	const SViewportBorder& topEdge = viewportArea.getTopEdge();
	const SViewportBorder& bottomEdge = viewportArea.getBottomEdge();

	const SViewportBorder& leftAxis = viewportArea.getLeftAxis();
	impl->selectSectPositions< SSelectHorzSectPositionsTraits >(
		viewportArea,
		leftAxis,
		topEdge,
		bottomEdge,
		impl->d_horzRoot,
		sectposids );

	const SViewportBorder& rightAxis = viewportArea.getRightAxis();
	impl->selectSectPositions< SSelectHorzSectPositionsTraits >(
		viewportArea,
		rightAxis,
		topEdge,
		bottomEdge,
		impl->d_horzRoot,
		sectposids );

	/*
		select sections crossing viewport rect
		interval inclined section is in fact visible by engine as 2x horz and 2x vert sections
		i.e. rect - there is no problem when at least one of them passes edge of viewport rect
		but it is possible that begin is in front of and above of top-left corner of viewport
		and end is behind and below the of bottom-right corner of viewport
		in such case rect of interval section fully covers viewport rect
		to select it find horz sections which passes left axis above of vieport rect and check
		whether its right-bottom corner is behind/below of viewport rect
	*/
	const SViewportBorder& mapTopBorder = viewportArea.getMapTopBorder();
	sect_pos_ids_t crosssectposids;
	impl->selectSectPositions< SSelectHorzSectPositionsTraits >(
		viewportArea,
		leftAxis,
		mapTopBorder,
		topEdge,
		impl->d_horzRoot,
		&crosssectposids );
	impl->selectCrossSections( viewportArea, crosssectposids, sectposids );

	// vertical-axis tree
	using is_in_front_of_vert_axis = is_in_left_top_side_range< utils::compare_by_y, SViewportBorder >;
	using is_behind_vert_axis = is_in_right_bottom_side_range< utils::compare_by_y, SViewportBorder >;

	using SSelectVertSectPositionsTraits = SSelectSectPositionsTraits<
		utils::compare_by_y,
		is_in_front_of_vert_axis,
		is_behind_vert_axis,
		utils::compare_by_x >;

	const SViewportBorder& leftEdge = viewportArea.getLeftEdge();
	const SViewportBorder& rightEdge = viewportArea.getRightEdge();

	const SViewportBorder& topAxis = viewportArea.getTopAxis();
	impl->selectSectPositions< SSelectVertSectPositionsTraits >(
		viewportArea,
		topAxis,
		leftEdge,
		rightEdge,
		impl->d_vertRoot,
		sectposids );

	// don't need to check sections passing bottom axis, they will be
	// detected in other searches, because they pass remaining edges or
	// have at least one end-point inside the viewport rect
	//const SViewportBorder& bottomAxis = viewportArea.getBottomAxis();
	//impl->selectSectPositions< SSelectVertSectPositionsTraits >(
	//	viewportArea,
	//	bottomAxis,
	//	leftEdge,
	//	rightEdge,
	//	impl->d_vertRoot,
	//	sectposids );
}

} // namespace be
