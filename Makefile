all:
	( . ./edksetup.sh && build --buildtarget=RELEASE --tagname=GCC5 --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 )
