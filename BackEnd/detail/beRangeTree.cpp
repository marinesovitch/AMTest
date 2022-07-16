// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beRangeTree.h"
#include "beSegmentsManager.h"
#include "beViewportArea.h"
#include "beTreeUtils.h"
#include "beUtils.h"
#include "beConfig.h"

namespace be
{

namespace
{

struct SRangeTreeItem;
struct SRangeTreeNodeBase;
struct SRangeTreeRoot;
struct SRangeTreeNode;
struct SRangeTreeLeaf;

class KRangeTreeItemVisitor
{
	protected:
		KRangeTreeItemVisitor();

	public:
		virtual ~KRangeTreeItemVisitor();

	public:
		virtual void visitNodeBase( SRangeTreeNodeBase* node );
		virtual void visitRoot( SRangeTreeRoot* root );
		virtual void visitNode( SRangeTreeNode* node );
		virtual void visitLeaf( SRangeTreeLeaf* leaf );
		virtual void visitDefault( SRangeTreeItem* item );

};		

// ----------------------------------------------------------------------------

struct SRangeTreeItem
{
	protected:
		SRangeTreeItem( const SPointPos* pointPos );

	public:
		virtual ~SRangeTreeItem();

	public:
		virtual bool isLeaf() const = 0;

		virtual SRangeTreeItem* getLeftChild() = 0;
		virtual SRangeTreeItem* getRightChild() = 0;

		virtual void accept( KRangeTreeItemVisitor* visitor ) = 0;

	public:
		const SPointPos* d_pointPos;
};

SRangeTreeItem::SRangeTreeItem( const SPointPos* pointPos )
	: d_pointPos( pointPos )
{
}

SRangeTreeItem::~SRangeTreeItem()
{
}

// ----------------------------------------------------------------------------

struct SRangeTreeNodeBase : public SRangeTreeItem
{
	public:
		SRangeTreeNodeBase( const SPointPos* pointPos );
		virtual ~SRangeTreeNodeBase();

	public:
		virtual bool isLeaf() const;

		virtual SRangeTreeItem* getLeftChild();
		virtual SRangeTreeItem* getRightChild();

		virtual void accept( KRangeTreeItemVisitor* visitor );

	public:
		SRangeTreeItem* d_leftChild;
		SRangeTreeItem* d_rightChild;
};

SRangeTreeNodeBase::SRangeTreeNodeBase( const SPointPos* pointPos ) 
	: SRangeTreeItem( pointPos )
	, d_leftChild( 0 )
	, d_rightChild( 0 )
{
}

SRangeTreeNodeBase::~SRangeTreeNodeBase()
{
	delete d_leftChild;
	delete d_rightChild;
}

bool SRangeTreeNodeBase::isLeaf() const
{
	return false;
}

SRangeTreeItem* SRangeTreeNodeBase::getLeftChild()
{
	return d_leftChild;
}

SRangeTreeItem* SRangeTreeNodeBase::getRightChild()
{
	return d_rightChild;
}

void SRangeTreeNodeBase::accept( KRangeTreeItemVisitor* visitor )
{
	visitor->visitNodeBase( this );
}

// ----------------------------------------------------------------------------

struct SRangeTreeRoot : public SRangeTreeNodeBase
{
	public:
		SRangeTreeRoot( const SPointPos* pointPos );

	public:
		virtual void accept( KRangeTreeItemVisitor* visitor );
};

SRangeTreeRoot::SRangeTreeRoot( const SPointPos* pointPos )
	: SRangeTreeNodeBase( pointPos )
{
}

void SRangeTreeRoot::accept( KRangeTreeItemVisitor* visitor )
{
	visitor->visitRoot( this );
}

// ----------------------------------------------------------------------------

struct SRangeTreeNode : public SRangeTreeNodeBase
{
	public:
		SRangeTreeNode( const SPointPos* pointPos );

	public:
		virtual void accept( KRangeTreeItemVisitor* visitor );

	public:
		point_positions_t d_point_positions_by_y;
};

SRangeTreeNode::SRangeTreeNode( const SPointPos* pointPos )
	: SRangeTreeNodeBase( pointPos )
{
}

void SRangeTreeNode::accept( KRangeTreeItemVisitor* visitor )
{
	visitor->visitNode( this );
}

// ----------------------------------------------------------------------------

struct SRangeTreeLeaf : public SRangeTreeItem
{
	public:
		SRangeTreeLeaf( const SPointPos* pointPos );

	public:
		virtual bool isLeaf() const;

		virtual SRangeTreeItem* getLeftChild();
		virtual SRangeTreeItem* getRightChild();

		virtual void accept( KRangeTreeItemVisitor* visitor );
};

SRangeTreeLeaf::SRangeTreeLeaf( const SPointPos* pointPos ) : SRangeTreeItem( pointPos )
{
}

bool SRangeTreeLeaf::isLeaf() const
{
	return true;
}

SRangeTreeItem* SRangeTreeLeaf::getLeftChild()
{
	return 0;
}

SRangeTreeItem* SRangeTreeLeaf::getRightChild()
{
	return 0;
}

void SRangeTreeLeaf::accept( KRangeTreeItemVisitor* visitor )
{
	visitor->visitLeaf( this );
}

// ----------------------------------------------------------------------------

KRangeTreeItemVisitor::KRangeTreeItemVisitor()
{
}

KRangeTreeItemVisitor::~KRangeTreeItemVisitor()
{
}

void KRangeTreeItemVisitor::visitNodeBase( SRangeTreeNodeBase* node )
{
	visitDefault( node );
}

void KRangeTreeItemVisitor::visitRoot( SRangeTreeRoot* root )
{
	visitNodeBase( root );
}

void KRangeTreeItemVisitor::visitNode( SRangeTreeNode* node )
{
	visitNodeBase( node );
}

void KRangeTreeItemVisitor::visitLeaf( SRangeTreeLeaf* leaf )
{
	visitDefault( leaf );
}

void KRangeTreeItemVisitor::visitDefault( SRangeTreeItem* /*item*/ )
{
	assert( !"unsupported item" );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

typedef std::set< const SPointPos* > pointpos_set_t;
typedef pointpos_set_t::iterator pointpos_set_it;

struct SCheckResult
{
	SCheckResult( const point_positions_t& point_positions )
		: d_consistent( true )
		, d_pointpos_set( point_positions.begin(), point_positions.end() )
	{
	}

	bool d_consistent;
	pointpos_set_t d_pointpos_set;
};

class KCheckRangeTreeConsistency : public KRangeTreeItemVisitor
{
	public:
		KCheckRangeTreeConsistency( 
			const SRangeTreeNodeBase* parent, 
			const EChildSide childSide,
			SCheckResult* result );

	public:
		static bool run(
			SRangeTreeItem* root,
			const point_positions_t& point_positions );

	public:
		virtual void visitNodeBase( SRangeTreeNodeBase* node );
		virtual void visitLeaf( SRangeTreeLeaf* leaf );
		virtual void visitDefault( SRangeTreeItem* item );

	private:
		const SRangeTreeNodeBase* d_parent;
		const EChildSide d_childSide;

		SCheckResult* d_result;

};

// ----------------------------------------------------------------------------

KCheckRangeTreeConsistency::KCheckRangeTreeConsistency( 
	const SRangeTreeNodeBase* parent,
	const EChildSide childSide,
	SCheckResult* result )
	: d_parent( parent )
	, d_childSide( childSide )
	, d_result( result )
{
}

bool KCheckRangeTreeConsistency::run( 
	SRangeTreeItem* root,
	const point_positions_t& point_positions )
{
	bool result = true;
	#ifdef ENABLE_TREE_CHECKERS
	if ( root != 0 )
	{
		SCheckResult checkResult( point_positions );
		KCheckRangeTreeConsistency checkConsistency( 0, NoParent, &checkResult );
		root->accept( &checkConsistency );
		result = checkResult.d_consistent && checkResult.d_pointpos_set.empty();
	}
	#endif
	return result;
}

void KCheckRangeTreeConsistency::visitNodeBase( SRangeTreeNodeBase* node )
{
	visitDefault( node );

	SRangeTreeItem* leftChild = node->getLeftChild();
	if ( leftChild != 0 )
	{
		KCheckRangeTreeConsistency checkConsistency( node, LeftChild, d_result );
		leftChild->accept( &checkConsistency );
	}

	SRangeTreeItem* rightChild = node->getRightChild();
	if ( rightChild != 0 )
	{
		KCheckRangeTreeConsistency checkConsistency( node, RightChild, d_result );
		rightChild->accept( &checkConsistency );
	}
}

void KCheckRangeTreeConsistency::visitLeaf( SRangeTreeLeaf* leaf )
{
	visitDefault( leaf );

	pointpos_set_t& pointpos_set = d_result->d_pointpos_set;
	const SPointPos* pointPos = leaf->d_pointPos;
	pointpos_set_it it = d_result->d_pointpos_set.find( pointPos );
	assert( it != pointpos_set.end() );
	pointpos_set.erase( it );
}

void KCheckRangeTreeConsistency::visitDefault( SRangeTreeItem* item )
{
	if ( d_parent != 0 )
	{
		bool consistent = true;
		const SPointPos* parentPos = d_parent->d_pointPos;
		const SPointPos* itemPos = item->d_pointPos; 
		if ( d_childSide == LeftChild )
		{
			if ( !utils::less_by_x( itemPos, parentPos ) && ( itemPos != parentPos ) )
				consistent = false;
		}
		else
		{
			assert( d_childSide == RightChild );
			if ( !utils::less_by_x( parentPos, itemPos ) )
				consistent = false;
		}

		if ( !consistent )
		{
			d_result->d_consistent = false;
			assert( !"inconsistent tree!" ); // stopping assertion
		}
	}
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KCreateAssociatedStructure : public KRangeTreeItemVisitor
{
	public:
		KCreateAssociatedStructure(
			point_positions_cit begin,
			point_positions_cit end );

	public:
		virtual void visitRoot( SRangeTreeRoot* root );
		virtual void visitNode( SRangeTreeNode* node );

	private:
		point_positions_cit d_begin;
		point_positions_cit d_end;

};

KCreateAssociatedStructure::KCreateAssociatedStructure(
	point_positions_cit begin,
	point_positions_cit end )
	: d_begin( begin )
	, d_end( end )
{
}

void KCreateAssociatedStructure::visitRoot( SRangeTreeRoot* /*root*/ )
{
	// don't create associated structure for root
}

void KCreateAssociatedStructure::visitNode( SRangeTreeNode* node )
{
	point_positions_t point_positions_by_y( d_begin, d_end );
	std::sort( 
		point_positions_by_y.begin(), 
		point_positions_by_y.end(), 
		utils::compare_by_y() );

	node->d_point_positions_by_y.swap( point_positions_by_y );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KRangeTreeBuilder
{
	public:
		KRangeTreeBuilder();

	public:
		SRangeTreeItem* run( const point_positions_t& point_positions_by_x );

	private:
		template< typename TTreeNode >
		SRangeTreeItem* createItem(
			point_positions_cit begin,
			point_positions_cit end );

		template< typename TTreeNode >
		SRangeTreeItem* createNode(
			point_positions_cit begin,
			point_positions_cit v_split_node_it,
			point_positions_cit end );

		SRangeTreeItem* createLeaf( 
			const SPointPos* pointPos );

};

// ----------------------------------------------------------------------------

KRangeTreeBuilder::KRangeTreeBuilder()
{
}

SRangeTreeItem* KRangeTreeBuilder::run( const point_positions_t& point_positions_by_x )
{
	point_positions_cit begin = point_positions_by_x.begin();
	point_positions_cit end = point_positions_by_x.end();
	SRangeTreeItem* root = createItem< SRangeTreeRoot >( begin, end );
	return root;
}

// ----------------------------------------------------------------------------

template< typename TTreeNode >
SRangeTreeItem* KRangeTreeBuilder::createItem(
	point_positions_cit begin,
	point_positions_cit end )
{
	SRangeTreeItem* result = 0;
	const std::size_t subtreeNodeCount = std::distance( begin, end );
	if ( 1 < subtreeNodeCount )
	{
		point_positions_cit v_split_node_it = utils::get_median( begin, end );
		assert( v_split_node_it != end );
		result = createNode< TTreeNode >( begin, v_split_node_it, end );
	}
	else if ( subtreeNodeCount == 1 )
	{
		const SPointPos* pointPos = *begin;
		result = createLeaf( pointPos );
	}
	return result;
}

template< typename TTreeNode >
SRangeTreeItem* KRangeTreeBuilder::createNode(
	point_positions_cit begin,
	point_positions_cit v_split_node_it,
	point_positions_cit end )
{
	const SPointPos* pointPos = *v_split_node_it;
	SRangeTreeNodeBase* node = new TTreeNode( pointPos );

	KCreateAssociatedStructure createAssociatedStructure( begin, end );
	node->accept( &createAssociatedStructure );

	point_positions_cit lbegin = begin;
	point_positions_cit lend = v_split_node_it + 1;
	node->d_leftChild = createItem< SRangeTreeNode >( lbegin, lend );

	point_positions_cit rbegin = lend;
	point_positions_cit rend = end;
	node->d_rightChild = createItem< SRangeTreeNode >( rbegin, rend );

	return node;
}

SRangeTreeItem* KRangeTreeBuilder::createLeaf( const SPointPos* pointPos )
{
	SRangeTreeLeaf* result = new SRangeTreeLeaf( pointPos );
	return result;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KSelectSubtreePoints : public KRangeTreeItemVisitor
{
	public:
		KSelectSubtreePoints( 
			const KViewportArea& viewportArea,
			point_ids_t* pointids );

	public:
		virtual void visitNode( SRangeTreeNode* node );
		virtual void visitDefault( SRangeTreeItem* item );

	private:
		void addPoint( const SPointPos* pointPos );

	private:
		const KViewportArea& d_viewportArea;
		point_ids_t& d_pointids;

};		

KSelectSubtreePoints::KSelectSubtreePoints(
	const KViewportArea& viewportArea,
	point_ids_t* pointids )
	: d_viewportArea( viewportArea )
	, d_pointids( *pointids )
{
}

void KSelectSubtreePoints::visitNode( SRangeTreeNode* node )
{
	const point_positions_t& point_positions_by_y = node->d_point_positions_by_y;

	const SViewportBorder& topEdge = d_viewportArea.getTopEdge();
	point_positions_cit begin 
		= std::lower_bound( 
			point_positions_by_y.begin()
			, point_positions_by_y.end()
			, topEdge
			, utils::compare_by_y() );

	const SViewportBorder& bottomEdge = d_viewportArea.getBottomEdge();
	point_positions_cit end 
		= std::upper_bound( 
			point_positions_by_y.begin()
			, point_positions_by_y.end()
			, bottomEdge
			, utils::compare_by_y() );

	for ( point_positions_cit it = begin
		; it != end
		; ++it )
	{
		const SPointPos* pointPos = *it;
		assert( d_viewportArea.contains( pointPos->d_point ) );
		addPoint( pointPos );
	}
}

void KSelectSubtreePoints::visitDefault( SRangeTreeItem* item )
{
	const SPointPos* pointPos = item->d_pointPos;
	const SPoint& point = pointPos->d_point;
	if ( d_viewportArea.contains( point ) )
		addPoint( pointPos );
}

void KSelectSubtreePoints::addPoint( const SPointPos* pointPos )
{
	assert( d_viewportArea.contains( pointPos->d_point ) );
	const point_pos_id_t pointid = pointPos->d_id;
	d_pointids.push_back( pointid );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KSelectPoints
{
	public:
		KSelectPoints(
			SRangeTreeItem* root,
			const KViewportArea& viewportArea,
			point_ids_t* pointids );

	public:
		void run();

	private:
		SRangeTreeItem* findSplitNode();
		void traverseSubtree( SRangeTreeItem* node );
		void traverseLeftSubtree(
			SRangeTreeItem* subtreeRoot,
			KSelectSubtreePoints* selectSubtreePoints );
		void traverseRightSubtree(
			SRangeTreeItem* subtreeRoot,
			KSelectSubtreePoints* selectSubtreePoints );
		void addSectPosIfInArea( SRangeTreeItem* node );

	private:
		SRangeTreeItem* d_root;
		const KViewportArea& d_viewportArea;
		point_ids_t& d_pointids;

};

// ----------------------------------------------------------------------------

KSelectPoints::KSelectPoints(
	SRangeTreeItem* root,
	const KViewportArea& viewportArea,
	point_ids_t* pointids )
	: d_root( root )
	, d_viewportArea( viewportArea )
	, d_pointids( *pointids )
{
}

void KSelectPoints::run()
{
	SRangeTreeItem* splitNode = findSplitNode();
	if ( splitNode != 0 )
		traverseSubtree( splitNode );
}

SRangeTreeItem* KSelectPoints::findSplitNode()
{
	SRangeTreeItem* result = 0;

	const SViewportBorder& leftEdge = d_viewportArea.getLeftEdge();
	const SViewportBorder& rightEdge = d_viewportArea.getRightEdge();

	SRangeTreeItem* node = d_root;
	while ( node != 0 )
	{
		const SPointPos* pointPos = node->d_pointPos;
		if ( utils::less_by_x( pointPos, leftEdge ) )
		{
			node = node->getRightChild();
		}
		else if ( utils::less_by_x( rightEdge, pointPos ) )
		{
			node = node->getLeftChild();
		}
		else
		{
			assert( utils::less_by_x( leftEdge, pointPos )
				&& utils::less_by_x( pointPos, rightEdge ) );
			result = node;
			node = 0;
		}
	}

	return result;
}

void KSelectPoints::traverseSubtree( SRangeTreeItem* subtreeRoot )
{
	if ( subtreeRoot->isLeaf() )
	{
		addSectPosIfInArea( subtreeRoot );
	}
	else
	{
		KSelectSubtreePoints selectSubtreePoints( d_viewportArea, &d_pointids );
		traverseLeftSubtree( subtreeRoot, &selectSubtreePoints );
		traverseRightSubtree( subtreeRoot, &selectSubtreePoints );
	}
}

void KSelectPoints::traverseLeftSubtree( 
	SRangeTreeItem* subtreeRoot,
	KSelectSubtreePoints* selectSubtreePoints )
{
	SRangeTreeItem* node = subtreeRoot->getLeftChild();
	const SViewportBorder& leftEdge = d_viewportArea.getLeftEdge();
	while ( ( node != 0 ) && !node->isLeaf() )
	{
		const SPointPos* pointPos = node->d_pointPos;
		if ( utils::less_by_x( leftEdge, pointPos ) )
		{
			SRangeTreeItem* rightChild = node->getRightChild();
			if ( rightChild != 0 )
				rightChild->accept( selectSubtreePoints );
			node = node->getLeftChild();
		}
		else
		{
			node = node->getRightChild();
		}
	}
	addSectPosIfInArea( node );
}

void KSelectPoints::traverseRightSubtree( 
	SRangeTreeItem* subtreeRoot,
	KSelectSubtreePoints* selectSubtreePoints )
{
	SRangeTreeItem* node = subtreeRoot->getRightChild();
	const SViewportBorder& rightEdge = d_viewportArea.getRightEdge();
	while ( ( node != 0 ) && !node->isLeaf() )
	{
		const SPointPos* pointPos = node->d_pointPos;
		if ( utils::less_by_x( pointPos, rightEdge ) )
		{
			SRangeTreeItem* leftChild = node->getLeftChild();
			if ( leftChild != 0 )
				leftChild->accept( selectSubtreePoints );
			node = node->getRightChild();
		}
		else
		{
			node = node->getLeftChild();
		}
	}
	addSectPosIfInArea( node );
}

void KSelectPoints::addSectPosIfInArea( SRangeTreeItem* node )
{
	if ( ( node != 0 ) && node->isLeaf() )
	{
		const SPointPos* pointPos = node->d_pointPos;
		const SPoint& nodePoint = pointPos->d_point;
		if ( d_viewportArea.contains( nodePoint ) )
		{
			const point_pos_id_t pointid = pointPos->d_id; 
			d_pointids.push_back( pointid );
		}
	}
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

struct KRangeTree::Impl
{
	Impl( SRangeTreeItem* root );
	~Impl();

	SRangeTreeItem* d_root;
};

KRangeTree::Impl::Impl( SRangeTreeItem* root ) 
	: d_root( root )
{
}

KRangeTree::Impl::~Impl()
{
	delete d_root;
}

KRangeTree::KRangeTree( const KSegmentsManager& segmentsManager ) : impl( 0 )
{
	point_positions_t point_positions;
	if ( segmentsManager.getPointPositions( &point_positions ) )
	{
		std::sort( point_positions.begin(), point_positions.end(), utils::compare_by_x() );
		assert( std::adjacent_find( 
			point_positions.begin(), point_positions.end() ) == point_positions.end() ); // items should be unique
		KRangeTreeBuilder treeBuilder;
		SRangeTreeItem* root = treeBuilder.run( point_positions );
		assert( KCheckRangeTreeConsistency::run( root, point_positions ) );
		impl = new Impl( root );
	}
}

KRangeTree::~KRangeTree()
{
	delete impl;
}

void KRangeTree::selectPoints(
	const KViewportArea& viewportArea,
	point_ids_t* pointids ) const
{
	if ( impl != 0 )
	{
		SRangeTreeItem* root = impl->d_root;
		assert( root != 0 );
		KSelectPoints selectPoints( root, viewportArea, pointids );
		selectPoints.run();
	}
}

} // namespace be
