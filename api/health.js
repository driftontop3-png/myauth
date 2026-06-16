import { ensureDb } from "../lib/db.js";
import { sendJson } from "../lib/utils.js";

export default async function handler(req, res) {
  if (req.method !== "GET") {
    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  }

  try {
    await ensureDb();
    sendJson(res, 200, {
      success: true,
      message: "Astryn Resell Portal API is online",
      database: "connected",
    });
  } catch (error) {
    sendJson(res, 500, {
      success: false,
      message: "Database not configured. Add Vercel Postgres and set POSTGRES_URL.",
      error: error.message,
    });
  }
}
