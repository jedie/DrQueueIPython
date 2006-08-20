#!/bin/bash
#
# $Id$
#

TOOL_NAME="mkrelease_tag"
SVK_BASE="//drqueue/remote"
SVN_BASE="https://ssl.drqueue.org/svn"
TMP_PATH="./tmp.checkout"

usage () {
	cat <<END

DrQueue's release-tag and related tarball builder.

Usage: $TOOL_NAME [-h] [-b <from_branch>] -s|-k -t <tag>

    -h will print this help 
    -s will use svn to do the work
    -k will use svk instead of svn
    -t <tag_name> will create the tag <tag_name> (from trunk if -b <branch> not specified)
    -b <branch> will use latest code on that branch to create the tag instead of the trunk

Example:

 $TOOL_NAME -k -b 0.60.x -t 0.60.1

END
}

show_report () {
    cat <<EOF

Report of variables to be used:

  Repository access through: "$REPO_TOOL"
  Release tag to be created: "$TAG"
  Branch that will be used to create that release tag: "${BRANCH:-"(none) trunk"}"

EOF
}

check_repo_tool () {
    TOOL=$(which $REPO_TOOL)
    if [ -z $TOOL ]; then
        echo The tool "$REPO_TOOL" could not be found in your path
        exit 1
    fi
    return 0
}

check_repo_branch_info () {
    $TOOL ls $REPO_BRANCH_PATH > /dev/null
    if [ $? -ne 0 ]; then
        echo "There was a problem getting info on $REPO_PATH with $TOOL"
        exit 1
    fi
    return 0
}

check_repo_branch () {
    if [ -z "$BRANCH" ]; then
        BRANCH="trunk"
    fi
    case $REPO_TOOL in
        svk ) REPO_BRANCH_PATH="$SVK_BASE/$BRANCH";;
        svn ) REPO_BRANCH_PATH="$SVN_BASE/$BRANCH";;
        *   ) echo "ERROR: Repo_tool unknown"
              exit 1;;
    esac
    check_repo_branch_info
    return 0
}
            
check_repo_tag_info () {
    $TOOL ls $REPO_TAG_PATH > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "The tag you're trying to create already exists"
        exit 1
    fi
    return 0
}

check_repo_tag () {
    case $REPO_TOOL in
        svk ) REPO_TAG_PATH="$SVK_BASE/$TAG";;
        svn ) REPO_TAG_PATH="$SVN_BASE/$TAG";;
        *   ) echo "ERROR: Repo_tool unknown"
              exit 1;;
    esac
    check_repo_tag_info
    return 0
}

check_options () {
    check_repo_tool
    check_repo_branch
    check_repo_tag
}

tmp_commit () {
    MSG="$1"
    FILES="$2"
    if [ -z "$MSG" ]; then
        echo "ERROR: No message given for commit"
        exit 1
    fi
    $TOOL ci -m "$MSG" $FILES
    if [ $? -ne 0 ]; then
        echo "ERROR: there was a problem commiting $FILES"
        exit 1
    fi
    return 0
}

change_common_h () {
    COMMON_H_PATH="$TMP_PATH/libdrqueue/common.h"
    if [ ! -w $COMMON_H_PATH ]; then
        echo "No common.h at $COMMON_H_PATH"
        exit 1
    fi
    awk '/VERSION/ { print $1,$2," \"SED_VERSION_CHANGE_ME\""; next; }; /.*/ { print }' $COMMON_H_PATH > $COMMON_H_PATH.tmp || die "ERROR with awk in change_common_h"
    sed -e "s/SED_VERSION_CHANGE_ME/$VERSION/g" $COMMON_H_PATH.tmp > $COMMON_H_PATH || die "ERROR with sed in change_common_h"
    rm -f $COMMON_H_PATH.tmp
    tmp_commit "New version number on common.h: $VERSION" $COMMON_H_PATH
    return 0
}

tmp_checkout () {
    $TOOL co $REPO_BRANCH_PATH $TMP_PATH > /dev/null
    if [ $? -ne 0 ]; then
        echo "Error checking out $REPO_BRANCH_PATH to $TMP_PATH"
        exit 1
    fi
    echo "Completed temporay checkout"
    return 0
}

tmp_remove () {
    unset SURE
    read -p "Path $TMP_PATH is going to be removed. Are you sure ? (y/n) " SURE
    if [ "x$SURE" != "xy" ]; then
        echo "Exiting..."
        exit 1
    fi
    rm -fR "$TMP_PATH"
    echo "Completed temporary path removal"
    return 0    
}

repo_create_tag () {
    $TOOL cp -m "New version tag created ($VERSION)" $REPO_BRANCH_PATH $REPO_TAG_PATH
    if [ $? -ne 0 ]; then
        echo "Error creating new tag $REPO_TAG_PATH from $REPO_BRANCH_PATH"
        exit 1
    fi
    echo "Completed creation of tag"
    return 0
}

build_tarball () {
    $TOOL co $REPO_TAG_PATH drqueue-$VERSION > /dev/null
    if [ $? -ne 0 ]; then
        echo ERROR: There was a problem checking out the tag $REPO_TAG_PATH
        exit 1
    fi
    tar zcvf drqueue.$VERSION.tgz --exclude="*/.svn" drqueue-$VERSION
    rm -fR drqueue-$VERSION
    return 0
}

while getopts ":hskb:t:" OPTION
  do
  case $OPTION in
      h ) usage
          exit 0;;
      s ) REPO_TOOL="svn";;
      k ) REPO_TOOL="svk";;
      b ) BRANCH="branches/$OPTARG";;
      t ) TAG="tags/$OPTARG"; VERSION="$OPTARG";;
      * ) echo "ERROR. Uknown option $OPTION";;
  esac
done

shift $(($OPTIND - 1))

if [ -z "$REPO_TOOL" -o -z "$TAG" ]; then
    echo "ERROR: Either -s|-k or -t <tag> have not been set"
    usage
    exit 1
fi

check_options
show_report

read -p "Are these variables correct ? (y/n) " SURE
if [ "x$SURE" != "xy" ]; then
    echo "Exiting..."
    exit 1
fi

tmp_checkout
change_common_h
repo_create_tag
tmp_remove
build_tarball

exit 1

# Create Changelog
read -p "Do you want to recreate the ChangeLog ? (y/n) " CHLG
if [ "$CHLG" != "n" ]; then
	echo -n "Creating ChangeLog... "
	svn log -v | ./svn-scripts/svn2cl.pl > ChangeLog # Dump log to ChangeLog
	echo "Created !"
fi

# Creating tag
read -p "Do you want to create a tag for this package ? (y/n) " CRTTAG
if [ "$CRTTAG" = "y" ]; then
	echo "Creating tag $VERSION"
	(cd ..; svn cp -m "Tag $VERSION created by build_release.sh" https://ssl.drqueue.org/svn/trunk \
                                                               https://ssl.drqueue.org/svn/tags/$VERSION)
	echo "Created !"
  # Update Revision
  echo "Updating Revision"
  svn update > Revision
  echo "Commiting new Revision"
  svn ci -m "New Revision commited by build_package.sh" Revision
fi

# Build package
echo "Building package"
make clean > /dev/null
(
cd ..
svn co https://ssl.drqueue.org/svn/tags/$VERSION drqueue-$VERSION
tar zcvf drqueue.$VERSION.tgz --exclude="*/.svn" drqueue-$VERSION
rm -fR drqueue-$VERSION
) > /dev/null

# Package ChangeLog
read -p "Do you want to create a package ChangeLog compared to previous tag ? (y/n) " CHLG
if [ "$CHLG" = "y" ]; then
	read -p "From tag ?: " OLDTAG
	OLDREV=`svn log --stop-on-copy https://ssl.drqueue.org/svn/tags/$OLDTAG | awk '/^r[0-9]+/ { sub (/r/,"",$1); print $1; }'`
	echo "Old Revision number: $OLDREV"
	echo "Creating ChangeLog.$VERSION"
	(svn log -v -r HEAD:$OLDREV | ./svn-scripts/svn2cl.pl > ../ChangeLog.$VERSION )
fi
