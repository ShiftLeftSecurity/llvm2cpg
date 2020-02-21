name := "llvm2cpgintegration"
ThisBuild / organization := "io.shiftleft"
ThisBuild / scalaVersion := "2.13.0"

val cpgVersion = "0.11.78"

ThisBuild / resolvers ++= Seq(
  Resolver.mavenLocal,
  Resolver.bintrayRepo("shiftleft", "maven"),
  "Sonatype OSS" at "https://oss.sonatype.org/content/repositories/public",
)

libraryDependencies ++= Seq(
  "io.shiftleft"       %% "semanticcpg"                      % cpgVersion,
  "org.scalatest"      %% "scalatest"                        % "3.0.8" % Test,
)


