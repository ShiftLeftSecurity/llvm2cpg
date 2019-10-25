name := "llvm2cpgintegration"
ThisBuild / organization := "io.shiftleft"
ThisBuild / scalaVersion := "2.12.8"

val cpgVersion      = "0.10.77"

ThisBuild / resolvers ++= Seq(
  Resolver.mavenLocal,
  "Sonatype OSS" at "https://oss.sonatype.org/content/repositories/public",
)

libraryDependencies ++= Seq(
  "io.shiftleft"       %% "semanticcpg"                      % cpgVersion,
  "org.scalatest"      %% "scalatest"                        % "3.0.3" % Test,
)


