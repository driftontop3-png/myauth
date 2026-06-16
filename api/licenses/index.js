import { sql } from "../../lib/db.js";
import {
  requireUser,
  toLicense,
  generateId,
  getLicensesForUser,
  getLicenseStats,
} from "../../lib/auth.js";
import { applyMask, formatExpiry, parseBody, sendJson } from "../../lib/utils.js";

export default async function handler(req, res) {
  try {
    const user = await requireUser(req, res);
    if (!user) return;

    if (req.method === "GET") {
      const search = String(new URL(req.url, "http://localhost").searchParams.get("search") || "")
        .trim()
        .toLowerCase();

      let licenses = await getLicensesForUser(user.id);
      if (search) {
        licenses = licenses.filter(
          (license) =>
            license.key.toLowerCase().includes(search) ||
            license.note.toLowerCase().includes(search) ||
            license.status.includes(search)
        );
      }

      const stats = await getLicenseStats(user.id);
      return sendJson(res, 200, { success: true, licenses, stats });
    }

    if (req.method === "POST") {
      const body = await parseBody(req);
      const amount = Number(body.amount || 1);
      const mask = String(body.mask || "ASTRYN-****-****-****").trim();
      const level = Number(body.level || 1);
      const expiryUnit = String(body.expiryUnit || "days");
      const expiryDuration = Number(body.expiryDuration || 30);
      const note = String(body.note || "").trim();
      const useLower = body.useLower !== false;
      const useUpper = body.useUpper !== false;

      if (amount < 1 || amount > 100) {
        return sendJson(res, 400, { success: false, message: "License amount must be between 1 and 100." });
      }
      if (amount > 1 && !mask.includes("*")) {
        return sendJson(res, 400, { success: false, message: "Mask must contain * when generating multiple keys." });
      }

      const expiryLabel = formatExpiry(expiryUnit, expiryDuration);
      const keys = [];
      const createdLicenses = [];

      for (let i = 0; i < amount; i += 1) {
        const id = generateId(12);
        const key = applyMask(mask, useLower, useUpper);
        keys.push(key);

        const inserted = await sql`
          INSERT INTO licenses (
            id, user_id, license_key, status, level,
            expiry_unit, expiry_duration, expiry_label, note
          ) VALUES (
            ${id}, ${user.id}, ${key}, 'unused', ${level},
            ${expiryUnit}, ${expiryDuration}, ${expiryLabel}, ${note}
          )
          RETURNING *
        `;
        createdLicenses.push(toLicense(inserted.rows[0]));
      }

      const licenses = await getLicensesForUser(user.id);
      const stats = await getLicenseStats(user.id);

      return sendJson(res, 201, {
        success: true,
        message: `Generated ${amount} license key${amount > 1 ? "s" : ""}.`,
        keys,
        licenses,
        stats,
      });
    }

    if (req.method === "DELETE") {
      const deleted = await sql`
        DELETE FROM licenses
        WHERE user_id = ${user.id} AND status = 'unused'
        RETURNING id
      `;

      const licenses = await getLicensesForUser(user.id);
      const stats = await getLicenseStats(user.id);

      return sendJson(res, 200, {
        success: true,
        message: deleted.rows.length
          ? `Deleted ${deleted.rows.length} unused license(s).`
          : "No unused licenses to delete.",
        removed: deleted.rows.length,
        licenses,
        stats,
      });
    }

    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  } catch (error) {
    sendJson(res, 500, { success: false, message: error.message || "License request failed." });
  }
}
